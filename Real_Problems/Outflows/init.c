/* ///////////////////////////////////////////////////////////////////// */
/*! 
  \file  
  \brief Contains basic functions for problem initialization.

  The init.c file collects most of the user-supplied functions useful 
  for problem configuration.
  It is automatically searched for by the makefile.

  \author A. Mignone (mignone@ph.unito.it)
  \date   Sepy 10, 2012
*/
/* ///////////////////////////////////////////////////////////////////// */
#include "pluto.h"
#include "pluto_usr.h"
#include "init_tools.h"
#include "read_grav_table.h"
#include "interpolation.h"
#include "abundances.h"
#include "accretion.h"
#include "clouds.h"
#include "grid_geometry.h"
#include "hot_halo.h"
#include "outflow.h"


/* ********************************************************************* */
void Init (double *v, double x1, double x2, double x3)
/*! 
 * The Init() function can be used to assign initial conditions as
 * as a function of spatial position.
 *
 * \param [out] v   a pointer to a vector of primitive variables
 * \param [in] x1   coordinate point in the 1st dimension
 * \param [in] x2   coordinate point in the 2nd dimension
 * \param [in] x3   coordinate point in the 3rdt dimension
 *
 * The meaning of x1, x2 and x3 depends on the geometry:
 * \f[ \begin{array}{cccl}
 *    x_1  & x_2    & x_3  & \mathrm{Geometry}    \\ \noalign{\medskip}
 *     \hline
 *    x    &   y    &  z   & \mathrm{Cartesian}   \\ \noalign{\medskip}
 *    R    &   z    &  -   & \mathrm{cylindrical} \\ \noalign{\medskip}
 *    R    & \phi   &  z   & \mathrm{polar}       \\ \noalign{\medskip}
 *    r    & \theta & \phi & \mathrm{spherical} 
 *    \end{array}
 *  \f]
 *
 * Variable names are accessed by means of an index v[nv], where
 * nv = RHO is density, nv = PRS is pressure, nv = (VX1, VX2, VX3) are
 * the three components of velocity, and so forth.
 *
 *********************************************************************** */
{

    int nv;
    double halo_primitives[NVAR], out_primitives[NVAR];
#if CLOUDS
    double cloud_primitives[NVAR];
#endif
    static int once01 = 0;


    /* Some things that only need to be done once */
    if (!once01) {

        /* Initialize base normalization struct */
        SetBaseNormalization();

        /* Set normalization factors for input parameters */
        SetIniNormalization();

        /* Set outflow geometry struct with parameters of cone */
        SetNozzleGeometry();

#if ACCRETION == YES
        /* Set outflow geometry struct with parameters of cone */
        SetAccretionPhysics();
#endif

        double dx;
        dx = FLOWAXIS((g_domEnd[IDIR] - g_domBeg[IDIR]) / NX1;,
                      (g_domEnd[JDIR] - g_domBeg[JDIR]) / NX2;,
                      (g_domEnd[KDIR] - g_domBeg[KDIR]) / NX3;);

        /* Print some data */
        OutflowPrimitives(out_primitives, ARG_FLOWAXIS(dx, 0), 0);
        HotHaloPrimitives(halo_primitives, ARG_FLOWAXIS(dx, 0));
        PrintInitData01(out_primitives, halo_primitives);

        /* Done once now */
        once01 = 1;
    }


    /* Initialize nozzle if we're in hemisphere around
     * nozzle inlet region, otherwise halo */

     if (InNozzleRegion(x1, x2, x3) || InNozzleCap(x1, x2, x3)) {

        OutflowPrimitives(out_primitives, x1, x2, x3, 0);
        HotHaloPrimitives(halo_primitives, x1, x2, x3);

        for (nv = 0; nv < NVAR; ++nv) {
            v[nv] = halo_primitives[nv] +
                    (out_primitives[nv] - halo_primitives[nv]) * Profile(x1, x2, x3);
        }
    }

#if INTERNAL_BOUNDARY == YES
    else if (InFlankRegion(x1, x2, x3)) {

        HotHaloPrimitives(halo_primitives, x1, x2, x3);

        for (nv = 0; nv < NVAR; ++nv) {
            v[nv] = halo_primitives[nv];
        }
    }
#endif

        /* Initialize halo. Hot, and warm, if included */
    else {

        /* First get primitives array for hot halo */
        HotHaloPrimitives(halo_primitives, x1, x2, x3);

#if CLOUDS && CLOUDS_MULTI == NO

        /* If we're in the domain of the clouds cube */
        if (CloudPrimitives(cloud_primitives, x1, x2, x3)){
          for (nv = 0; nv < NVAR; ++nv) v[nv] = cloud_primitives[nv];
        }
        /* If not a cloud pixel then use hot halo primitives*/
        else{
          for (nv = 0; nv < NVAR; ++nv) v[nv] = halo_primitives[nv];
        }

#else

        /* Hot halo */
        for (nv = 0; nv < NVAR; ++nv) v[nv] = halo_primitives[nv];
#endif

    }

#if COOLING
    g_minCoolingTemp = 1.e4;
    g_maxCoolingRate = 0.1;
#endif

#if PHYSICS == MHD || PHYSICS == RMHD

    v[BX1] = 0.0;
    v[BX2] = 0.0;
    v[BX3] = 0.0;

    v[AX1] = 0.0;
    v[AX2] = 0.0;
    v[AX3] = 0.0;

#endif

}



/* ********************************************************************* */
void Analysis (const Data *d, Grid *grid)
/*! 
 *  Perform runtime data analysis.
 *
 * \param [in] d the PLUTO Data structure
 * \param [in] grid   pointer to array of Grid structures  
 *
 *********************************************************************** */
{

    /* Accretion */

#if ACCRETION == YES
    SphericalAccretion(d, grid);

#if ACCRETION_OUTPUT == YES
    SphericalAccretionOutput();
#endif

#endif


}


#if PHYSICS == MHD
/* ********************************************************************* */
void BackgroundField (double x1, double x2, double x3, double *B0)
/*!
 * Define the component of a static, curl-free background 
 * magnetic field.
 *
 * \param [in] x1  position in the 1st coordinate direction \f$x_1\f$
 * \param [in] x2  position in the 2nd coordinate direction \f$x_2\f$
 * \param [in] x3  position in the 3rd coordinate direction \f$x_3\f$
 * \param [out] B0 array containing the vector componens of the background
 *                 magnetic field
 *********************************************************************** */
{
   B0[0] = 0.0;
   B0[1] = 0.0;
   B0[2] = 0.0;
}
#endif

/* ********************************************************************* */
void UserDefBoundary (const Data *d, RBox *box, int side, Grid *grid) 
/*! 
 *  Assign user-defined boundary conditions.
 *
 * \param [in,out] d  pointer to the PLUTO data structure containing
 *                    cell-centered primitive quantities (d->Vc) and 
 *                    staggered magnetic fields (d->Vs, when used) to 
 *                    be filled.
 * \param [in] box    pointer to a RBox structure containing the lower
 *                    and upper indices of the ghost zone-centers/nodes
 *                    or edges at which data values should be assigned.
 * \param [in] side   specifies the boundary side where ghost zones need
 *                    to be filled. It can assume the following 
 *                    pre-definite values: X1_BEG, X1_END,
 *                                         X2_BEG, X2_END, 
 *                                         X3_BEG, X3_END.
 *                    The special value side == 0 is used to control
 *                    a region inside the computational domain.
 * \param [in] grid  pointer to an array of Grid structures.
 *
 *********************************************************************** */
{
    int i, j, k, nv;
    double *x1, *x2, *x3;
    double vc;
    double out_primitives[NVAR], halo_primitives[NVAR], result[NVAR], mirror[NVAR];
    static int touch = 0;
#if ACCRETION == YES
    double ****Vc_new;
#endif

    /* These are the geometrical central points */
    x1 = grid[IDIR].x;
    x2 = grid[JDIR].x;
    x3 = grid[KDIR].x;

#if INTERNAL_BOUNDARY == YES
    if (side == 0) {    /* -- check solution inside domain -- */

        if (SphereIntersectsDomain(grid, nz.sph)) {

#if ACCRETION == YES
            /* Create buffer array for internal sink region solution */
            Vc_new = ARRAY_4D(NVAR, NX3_TOT, NX2_TOT, NX1_TOT, double);
#endif

            TOT_LOOP(k, j, i) {

                        if (InNozzleRegion(x1[i], x2[j], x3[k])) {

#if ACCRETION == YES
                            OutflowPrimitives(out_primitives, x1[i], x2[j], x3[k], ac.accr_rate);
#else
                            OutflowPrimitives(out_primitives, x1[i], x2[j], x3[k], 0);
#endif

                            for (nv = 0; nv < NVAR; ++nv) {
                                vc = d->Vc[nv][k][j][i];
                                d->Vc[nv][k][j][i] = vc + (out_primitives[nv] - vc) *
                                                          Profile(x1[i], x2[j], x3[k]);
                            }

                            d->flag[k][j][i] |= FLAG_INTERNAL_BOUNDARY;

                        } // InNozzleRegion


#if ACCRETION == YES
                        else if (InSinkRegion(x1[i], x2[j], x3[k])) {

#if SINK_METHOD == SINK_FREEFLOW

                            SphericalFreeflowInternalBoundary(d->Vc, i, j, k, x1, x2, x3, result);

#elif SINK_METHOD == SINK_VACUUM

                            VacuumInternalBoundary(result);

#elif SINK_METHOD == SINK_BONDI

                            BondiFlowInternalBoundary(x1[i], x2[j], x3[k], result);

#elif SINK_METHOD == SINK_FEDERRATH

                           /* Remove mass according to Federrath's sink particle method */

#endif

                            /* Copy results to temporary solution array */
                            for (nv = 0; nv < NVAR; ++nv) Vc_new[nv][k][j][i] = result[nv];

                            d->flag[k][j][i] |= FLAG_INTERNAL_BOUNDARY;

                        } // InSinkRegion

#else
                        else if (InFlankRegion(x1[i], x2[j], x3[k])){

                            HotHaloPrimitives(halo_primitives, x1[i], x2[j], x3[k]);

                            for (nv = 0; nv < NVAR; ++nv) {
                                d->Vc[nv][k][j][i] = halo_primitives[nv];
                            }
                            d->flag[k][j][i] |= FLAG_INTERNAL_BOUNDARY;

                        } // InFlankRegion
#endif

                    } // TOT_LOOP

#if ACCRETION == YES

            /* Copy solution over in case of using Spherical inward free-flowing broundary conditions */
            TOT_LOOP(k, j, i) {

                        if (InSinkRegion(x1[i], x2[j], x3[k])) {
                            for (nv = 0; nv < NVAR; ++nv) {
                                d->Vc[nv][k][j][i] = Vc_new[nv][k][j][i];
                            }
                        }

                    } // Update TOT_LOOP

            FreeArray4D((void *) Vc_new);
#endif

        } // SphereIntersectsDomain

    } // side == 0

#endif

    if (side == FLOWAXIS(X2_BEG, X3_BEG, X1_BEG)) {
        if (box->vpos == CENTER) {
            BOX_LOOP(box, k, j, i) {

                        HaloOuterBoundary(side, d, i, j, k, x1[i], x2[j], x3[k], &touch);

                    }
        } else if (box->vpos == X1FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X2FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X3FACE) {
            BOX_LOOP(box, k, j, i) { }
        }
    }

    if (side == X1_END) {  /* -- X1_END boundary -- */
        if (box->vpos == CENTER) {
            BOX_LOOP(box, k, j, i) {

                        HaloOuterBoundary(side, d, i, j, k, x1[i], x2[j], x3[k], &touch);

                    }
        } else if (box->vpos == X1FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X2FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X3FACE) {
            BOX_LOOP(box, k, j, i) { }
        }
    }


    if (side == FLOWAXIS(X3_BEG, X1_BEG, X2_BEG)) {
        if (box->vpos == CENTER) {
            BOX_LOOP(box, k, j, i) {

                        HaloOuterBoundary(side, d, i, j, k, x1[i], x2[j], x3[k], &touch);

                    }
        } else if (box->vpos == X1FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X2FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X3FACE) {
            BOX_LOOP(box, k, j, i) { }
        }
    }


    if (side == X2_END) {  /* -- X2_END boundary -- */
        if (box->vpos == CENTER) {
            BOX_LOOP(box, k, j, i) {

                        HaloOuterBoundary(side, d, i, j, k, x1[i], x2[j], x3[k], &touch);

                    }
        } else if (box->vpos == X1FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X2FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X3FACE) {
            BOX_LOOP(box, k, j, i) { }
        }
    }


    /* This side is where the nozzle is located and the outflow emerges,
     * unless internal boundary is set. */
    if (side == FLOWAXIS(X1_BEG, X2_BEG, X3_BEG)) {
        if (box->vpos == CENTER) {
            BOX_LOOP(box, k, j, i) {

                        /* If we're doing a full galaxy simulation,
                           as opposed to a half-galaxy simulation,
                           treat just like other _BEG boundaries    */

                        if (FLOWAXIS(g_domBeg[IDIR], g_domBeg[JDIR], g_domBeg[KDIR]) < 0.) {

                            HaloOuterBoundary(side, d, i, j, k, x1[i], x2[j], x3[k], &touch);

                        } // full galaxy

                        else { // half galaxy

                            /* Reflective boundary */
                            for (nv = 0; nv < NVAR; ++nv) {
                                mirror[nv] = d->Vc[nv]
                                FLOWAXIS([k][j][2 * IBEG - i - 1],
                                         [k][2 * JBEG - j - 1][i],
                                         [2 * KBEG - k - 1][j][i]);
                            }

                            mirror[FLOWAXIS(VX1, VX2, VX3)] *= -1.0;

                            if (InNozzleRegion(x1[i], x2[j], x3[k])) {

#if ACCRETION == YES
                                OutflowPrimitives(out_primitives, x1[i], x2[j], x3[k], ac.accr_rate);
#else
                                OutflowPrimitives(out_primitives, x1[i], x2[j], x3[k], 0);
#endif

                                for (nv = 0; nv < NVAR; ++nv) {
                                    d->Vc[nv][k][j][i] = mirror[nv] + (out_primitives[nv] - mirror[nv]) *
                                                                      Profile(x1[i], x2[j], x3[k]);
                                }

                            }
                            else {
                                for (nv = 0; nv < NVAR; ++nv) {
                                    d->Vc[nv][k][j][i] = mirror[nv];
                                }
                            }

                        } // half galaxy

                    } // BOX_LOOP
        } else if (box->vpos == X1FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X2FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X3FACE) {
            BOX_LOOP(box, k, j, i) { }
        }
    }

    if (side == X3_END) {  /* -- X3_END boundary -- */
        if (box->vpos == CENTER) {
            BOX_LOOP(box, k, j, i) {

                        HaloOuterBoundary(side, d, i, j, k, x1[i], x2[j], x3[k], &touch);

                    }
        } else if (box->vpos == X1FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X2FACE) {
            BOX_LOOP(box, k, j, i) { }
        } else if (box->vpos == X3FACE) {
            BOX_LOOP(box, k, j, i) { }
        }
    }
}


#if BODY_FORCE != NO
/* ********************************************************************* */
void BodyForceVector(double *v, double *g, double x1, double x2, double x3)
/*!
 * Prescribe the acceleration vector as a function of the coordinates
 * and the vector of primitive variables *v.
 *
 * \param [in] v  pointer to a cell-centered vector of primitive 
 *                variables
 * \param [out] g acceleration vector
 * \param [in] x1  position in the 1st coordinate direction \f$x_1\f$
 * \param [in] x2  position in the 2nd coordinate direction \f$x_2\f$
 * \param [in] x3  position in the 3rd coordinate direction \f$x_3\f$
 *
 *********************************************************************** */
{

    /* Common variables */
    double r,  gr;

    /* Hernquist potential */
#if GRAV_POTENTIAL == GRAV_HERNQUIST

    double a, rho0;
    r = SPH1(x1, x2, x3);
    a = g_inputParam[PAR_HRAD] * ini_code[PAR_HRAD];
    rho0 = g_inputParam[PAR_HRHO] * ini_code[PAR_HRHO];

    gr = -2. * CONST_PI * CONST_G / vn.newton_norm * rho0 * a / pow((1. + r / a), 2);


    /* Gravity Table */
#elif defined GRAV_TABLE

    r = SPH1(x1, x2, x3);
    gr = -InterpolationWrapper(gr_rad, gr_vec, gr_ndata, r);


    /* Flat thermodynamic profile but gravity is on.
     * The potential, thus, is a parabola */
#elif GRAV_POTENTIAL == GRAV_HOMOGENEOUS

    r = SPH1(x1, x2, x3);

    gr = -4 * CONST_PI * CONST_G / vn.newton_norm * g_inputParam[PAR_HRHO] * ini_code[PAR_HRHO] * r;

    /* No gravity */

#endif

    /* Add potential of point mass at center */
#if ACCRETION == YES

    double mbh_ini = g_inputParam[PAR_AMBH] * ini_code[PAR_AMBH];
    gr += CONST_G * (ac.mbh - mbh_ini) / (vn.newton_norm * r * r);

#endif


    double sx1, sx2, sx3;
    sx1 = SPH1(x1, x2, x3);
    sx2 = SPH2(x1, x2, x3);
    sx3 = SPH3(x1, x2, x3);

    /* Gravity pointing to (0,0,0) - possibly reconsider */
    g[IDIR] = VSPH_1(sx1, sx2, sx3, gr, 0, 0);
    g[JDIR] = VSPH_2(sx1, sx2, sx3, gr, 0, 0);
    g[KDIR] = VSPH_3(sx1, sx2, sx3, gr, 0, 0);

}

/* ********************************************************************* */
double BodyForcePotential(double x1, double x2, double x3)
/*!
 * Return the gravitational potential as function of the coordinates.
 *
 * \param [in] x1  position in the 1st coordinate direction \f$x_1\f$
 * \param [in] x2  position in the 2nd coordinate direction \f$x_2\f$
 * \param [in] x3  position in the 3rd coordinate direction \f$x_3\f$
 * 
 * \return The body force potential \f$ \Phi(x_1,x_2,x_3) \f$.
 *
 *********************************************************************** */
{

    /* Common variables */
    double r, pot;

    /* Hernquist potential */
#if GRAV_POTENTIAL == GRAV_HERNQUIST

    double a, rho0;

    r = SPH1(x1, x2, x3);
    a = g_inputParam[PAR_HRAD] * ini_code[PAR_HRAD];
    rho0 = g_inputParam[PAR_HRHO] * ini_code[PAR_HRHO];

    pot = 2. * CONST_PI * CONST_G / vn.newton_norm * rho0 * a * a / (1. + r / a);


    /* Gravity table */
#elif defined GRAV_TABLE

    double fc, y0, y1, y2, y3, r1, r2;

    r = SPH1(x1, x2, x3);
    pot = InterpolationWrapper(gr_rad, gr_phi, gr_ndata, r);


    /* Flat thermodynamic profile but gravity is on.
     * The potential, thus, is a parabola */
#elif GRAV_POTENTIAL == GRAV_HOMOGENEOUS

    r = SPH1(x1, x2, x3);
    pot = 2 * CONST_PI * CONST_G / vn.newton_norm * g_inputParam[PAR_HRHO] * ini_code[PAR_HRHO] * r * r;

#endif


    /* Add potential of point mass at center */
#if ACCRETION == YES

    double mbh_ini = g_inputParam[PAR_AMBH] * ini_code[PAR_AMBH];
    pot -= CONST_G * (ac.mbh - mbh_ini) / (vn.newton_norm * r);

#endif

    return pot;

}

#endif

