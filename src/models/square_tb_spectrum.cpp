#include "models/square_tb_spectrum.h"

namespace anomtrans {

square_tb_Hamiltonian::square_tb_Hamiltonian(double _t, double _tp, kmBasis<2> _kmb)
      : t(_t), tp(_tp), kmb(_kmb) {}

double square_tb_Hamiltonian::energy(kmComps<2> ikm_comps) const {
  kmVals<2> km = kmb.km_at(ikm_comps);
  kVals<2> k = std::get<0>(km);
  unsigned int m = std::get<1>(km);

  if (m != 0) {
    throw std::invalid_argument("square_tb_Hamiltonian is not defined for Nbands > 1");
  }

  double kx = 2*pi*k.at(0);
  double ky = 2*pi*k.at(1);

  return -2*t*(std::cos(kx) + std::cos(ky)) + 4*tp*std::cos(kx)*std::cos(ky);
}

std::array<std::complex<double>, 2> square_tb_Hamiltonian::gradient(kmComps<2> ikm_comps, unsigned int mp) const {
  kmVals<2> km = kmb.km_at(ikm_comps);
  kVals<2> k = std::get<0>(km);
  unsigned int m = std::get<1>(km);

  if (m != 0 or mp != 0) {
    throw std::invalid_argument("square_tb_Hamiltonian is not defined for Nbands > 1");
  }

  double kx = 2*pi*k.at(0);
  double ky = 2*pi*k.at(1);

  // Since we only have one band, mp == m and the gradient <km|\grad_k H_k|km>
  // is the same as the derivative of the energy.
  double vx = 2*t*std::sin(kx) - 4*tp*std::sin(kx)*std::cos(ky);
  double vy = 2*t*std::sin(ky) - 4*tp*std::cos(kx)*std::sin(ky);

  std::array<std::complex<double>, 2> v = {vx, vy};
  return v;
}

std::complex<double> square_tb_Hamiltonian::basis_component(const PetscInt ikm, const unsigned int i) const {
  return std::complex<double>(1.0, 0.0);
}

} // namespace anomtrans
