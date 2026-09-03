#include <assert.h>
#include <hd.h>

void double_couple_point(theta_couple_point_t *out,
                         const theta_couple_point_t *in,
                         const theta_couple_curve_t *E1E2) {
  ec_dbl(&out->P1, &E1E2->E1, &in->P1);
  ec_dbl(&out->P2, &E1E2->E2, &in->P2);
}

void double_couple_point_iter(theta_couple_point_t *out, unsigned n,
                              const theta_couple_point_t *in,
                              const theta_couple_curve_t *E1E2) {
  if (n == 0) {
    memmove(out, in, sizeof(theta_couple_point_t));
  } else {
    double_couple_point(out, in, E1E2);
    for (unsigned i = 0; i < n - 1; i++) {
      double_couple_point(out, out, E1E2);
    }
  }
}

void add_couple_jac_points(theta_couple_jac_point_t *out,
                           const theta_couple_jac_point_t *T1,
                           const theta_couple_jac_point_t *T2,
                           const theta_couple_curve_t *E1E2) {
  ADD(&out->P1, &T1->P1, &T2->P1, &E1E2->E1);
  ADD(&out->P2, &T1->P2, &T2->P2, &E1E2->E2);
}

void double_couple_jac_point(theta_couple_jac_point_t *out,
                             const theta_couple_jac_point_t *in,
                             const theta_couple_curve_t *E1E2) {
  DBL(&out->P1, &in->P1, &E1E2->E1);
  DBL(&out->P2, &in->P2, &E1E2->E2);
}

void double_couple_jac_point_iter(theta_couple_jac_point_t *out, unsigned n,
                                  const theta_couple_jac_point_t *in,
                                  const theta_couple_curve_t *E1E2) {
  if (n == 0) {
    *out = *in;
  } else {
    double_couple_jac_point(out, in, E1E2);
    for (unsigned i = 0; i < n - 1; i++) {
      double_couple_jac_point(out, out, E1E2);
    }
  }
}

void couple_jac_to_xz(theta_couple_point_t *P,
                      const theta_couple_jac_point_t *xyP) {
  jac_to_xz(&P->P1, &xyP->P1);
  jac_to_xz(&P->P2, &xyP->P2);
}

void copy_bases_to_kernel(theta_kernel_couple_points_t *ker,
                          const ec_basis_t *B1, const ec_basis_t *B2) {
  // Copy the basis on E1 to (P, _) on T1, T2 and T1 - T2
  ker->T1.P1 = B1->P;
  ker->T2.P1 = B1->Q;
  ker->T1m2.P1 = B1->PmQ;

  // Copy the basis on E2 to (_, P) on T1, T2 and T1 - T2
  ker->T1.P2 = B2->P;
  ker->T2.P2 = B2->Q;
  ker->T1m2.P2 = B2->PmQ;
}
