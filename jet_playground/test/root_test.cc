#include <iostream>

#include "TLorentzVector.h"


int main() {
  
  TLorentzVector a( 1, 2, 3, 4 );
  
  std::cout << "vector a: { ";
  for ( int i = 0; i < 4; ++i ) { std::cout << a[i]; if ( i != 3 ) std::cout << ","; std::cout << " "; }
  std::cout <<" }" << std::endl;
  
  return 0;
}
