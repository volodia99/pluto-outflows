#define  PHYSICS                 HD
#define  DIMENSIONS              2
#define  COMPONENTS              2
#define  GEOMETRY                CARTESIAN
#define  BODY_FORCE              NO
#define  COOLING                 NO
#define  INTERPOLATION           LINEAR
#define  TIME_STEPPING           HANCOCK
#define  DIMENSIONAL_SPLITTING   NO
#define  NTRACER                 0
#define  USER_DEF_PARAMETERS     6
#define  USER_DEF_CONSTANTS      3

/* -- physics dependent declarations -- */

#define  EOS                     IDEAL
#define  ENTROPY_SWITCH          NO
#define  THERMAL_CONDUCTION      EXPLICIT
#define  VISCOSITY               NO
#define  ROTATING_FRAME          NO

/* -- user-defined parameters (labels) -- */

#define  BMAG                    0
#define  THETA                   1
#define  T_IN                    2
#define  T_OUT                   3
#define  RHO_IN                  4
#define  RHO_OUT                 5

/* -- user-defined symbolic constants -- */

#define  UNIT_DENSITY            (1.26*CONST_mH)
#define  UNIT_LENGTH             (CONST_pc)
#define  UNIT_VELOCITY           sqrt(g_gamma*CONST_kB*1.e6/(1.26*CONST_mH))

/* -- supplementary constants (user editable) -- */ 

#define  INITIAL_SMOOTHING      NO
#define  WARNING_MESSAGES       YES
#define  PRINT_TO_FILE          YES
#define  INTERNAL_BOUNDARY      NO
#define  SHOCK_FLATTENING       MULTID
#define  ARTIFICIAL_VISCOSITY   NO
#define  CHAR_LIMITING          NO
#define  LIMITER                VANLEER_LIM
#define  PRIMITIVE_HANCOCK      YES
