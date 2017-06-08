//
// Created by Alexander Y. Wagner on 4/6/16.
//

#ifndef PLUTO_ACCRETION_H
#define PLUTO_ACCRETION_H

#include "init_tools.h"
#include "outflow.h"

/* Structure for accretion physics. All are kept in code units. */
typedef struct {
    double rad;               // Radius of surface through which accretion rate is measured
    double mbh;               // BH mass
    double edd;               // Eddington power
    double eff;               // Fraction of accretion rate going into outflow
    double mld;               // Mass loading efficiency
    double snk;               // Sink radius
    double area;              // Area of surface
    double accr_rate_sel;     // Mass accretion rate, cell selection method
    double accr_rate_rss;     // Mass accretion rate, random spherical sampling
    double accr_rate_sel_28;     // Mass accretion rate, cell selection method
    double accr_rate_rss_28;     // Mass accretion rate, random spherical sampling
    double accr_rate_sel_29;     // Mass accretion rate, cell selection method
    double accr_rate_rss_29;     // Mass accretion rate, random spherical sampling
    double accr_rate_sel_30;     // Mass accretion rate, cell selection method
    double accr_rate_rss_30;     // Mass accretion rate, random spherical sampling
    double accr_rate_sel_35;     // Mass accretion rate, cell selection method
    double accr_rate_rss_35;     // Mass accretion rate, random spherical sampling
    double accr_rate_bondi;   // Bondi accretion rate
    double deboost;           // Deboost factor for outflow
    Nozzle nzi;               // Initial nozzle parameters
    OutflowState osi;         // Initial outflow state parameters

} Accretion;

extern Accretion ac;


void SetAccretionPhysics();

int InSinkRegion(const double x1, const double x2, const double x3);

void SphericalFreeflow(double *prims, double ****VC, const double *x1, const double *x2, const double *x3,
                       const int k, const int j, const int i);

void SphericalFreeflowInternalBoundary(const double ****Vc, int i, int j, int k, const double *x1, const double *x2,
                                       const double *x3, double *result);

void FederrathAccretion(const Data *d, Grid *grid);


double BondiAccretionRate(const double mbh, const double rho_far, const double snd_far);

double BondiAccretionRateLocal(const double mbh, const double rho_acc, const double snd_acc, const double snd_far);

double BondiLambda();

double BondiRadius(double m, double *v);

void BondiFlowInternalBoundary(const double x1, const double x2, const double x3, double *result);


void VacuumInternalBoundary(double *result);

double EddingtonLuminosity(const double mbh);

void TotalMass(const Data *d, Grid *grid);

double BondiAccretion(const Data *d, Grid *grid);

void SphericalAccretion(const Data *d, Grid *grid);

double SphericalSampledAccretion(const Data *d, Grid *grid, const double radius);

double SphericalSelectedAccretion(const Data *d, Grid *grid, const double radius);

void SphericalAccretionOutput();

double VirialParameter(const double * prim, const double mass,
                       const double x1, const double x2, const double x3);

double GravitationallyBound(const double *prim, const double mass, const double vol,
                            const double x1, const double x2, const double x3);

double JeansResolvedDensity(const double *prim);

double FederrathSinkInternalBoundary(const double ****Vc, int i, int j, int k, const double *x1, const double *x2,
                                     const double *x3, const double *dV1, const double *dV2, const double *dV3,
                                     double *result);

#endif //PLUTO_ACCRETION_H
