#include <iostream>
#include <math.h>

#include "TStarJetPicoReader.h"
#include "TStarJetPicoEvent.h"
#include "TStarJetPicoPrimaryTrack.h"
int main() {
  
  TStarJetPicoReader reader;
  TStarJetPicoPrimaryTrack a;
  
  std::cout << "track a pt: " << sqrt( a.GetPx()*a.GetPx() + a.GetPy()*a.GetPy() ) << std::endl;
  
  
  return 0;
  
}
