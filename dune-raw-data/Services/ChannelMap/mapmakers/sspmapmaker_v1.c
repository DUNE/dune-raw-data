/***
 *
 * Author: Alex Himmmel, Tom Junk
 *
 * HORRENDOUS hack -- copy-pasted most contents from dunetpc/dune/Geometry/ProtoDUNEChannelMapAlg.h
 *
 */

#include <stdio.h>
#include <map>
#include <iostream>
#include <iomanip>

typedef unsigned int Index;

std::map<Index, Index> fSSP;
std::map<Index, Index> fSSPChOne;
std::map<Index, Index> fOpDet;
std::map<Index, Index> fHWChannel;

int fMaxOpChannel;
int fNOpChannels;


Index NOpChannels(Index /*NOpDets*/);
Index MaxOpChannel(Index /*NOpDets*/);
Index NOpHardwareChannels(Index opDet);
Index OpChannel(Index detNum, Index channel);
Index OpDetFromOpChannel(Index opChannel);
Index HardwareChannelFromOpChannel(Index opChannel);
void  SSPandCh(Index detNum, Index channel, Index &ssp, Index &sspch);
Index OpChannelFromSSP(Index ssp, Index sspch);
bool  IsValidOpChannel(Index opChannel, Index /*NOpDets*/);

void PrintChannelMaps();


/* This assumes the first 10 channels of each SSP are wired up with channels and the last two
   are empty.  Assumes channels increase with Y */

void sspmapmaker_v1()
{
  
  
  
  // Manually entered based on maps from Chris Macias
  // http://indico.fnal.gov/event/14582/session/30/contribution/30/material/slides/0.pdf
  //
  // That gives SSP and channel ranges from OpDet. These offline channels correspond
  // to the APAs listed. Channel numbers increase by 2 going from top to bottom
  //
  //   USDaS   MSDaS  DSDaS
  //   41-59   21-39  1-19
  //
  //   USRaS   MSRaS  DSRaS
  //   40-58   20-38  0-18


  // DSDaS -- "normal" pattern
  fSSP[ 1] = 44; fSSPChOne[ 1] = 0;
  fSSP[ 3] = 43; fSSPChOne[ 3] = 8;
  fSSP[ 5] = 43; fSSPChOne[ 5] = 4;
  fSSP[ 7] = 43; fSSPChOne[ 7] = 0;
  fSSP[ 9] = 42; fSSPChOne[ 9] = 8;
  fSSP[11] = 42; fSSPChOne[11] = 4;
  fSSP[13] = 42; fSSPChOne[13] = 0;
  fSSP[15] = 41; fSSPChOne[15] = 8;
  fSSP[17] = 41; fSSPChOne[17] = 4;
  fSSP[19] = 41; fSSPChOne[19] = 0;

  // MSDaS -- unusual pattern for ARAPUCA
  // OpDet 31 is an ARAPUCA with 12 channels
  // Crosses into SSP 63, channels 0-3
  // Needs special casing later on
  fSSP[21] = 64; fSSPChOne[21] = 8;
  fSSP[23] = 64; fSSPChOne[23] = 4;
  fSSP[25] = 64; fSSPChOne[25] = 0;
  fSSP[27] = 63; fSSPChOne[27] = 8;
  fSSP[29] = 63; fSSPChOne[29] = 4;
  fSSP[31] = 62; fSSPChOne[31] = 4; 
  fSSP[33] = 62; fSSPChOne[33] = 0;
  fSSP[35] = 61; fSSPChOne[35] = 8;
  fSSP[37] = 61; fSSPChOne[37] = 4;
  fSSP[39] = 61; fSSPChOne[39] = 0;

  // USDaS -- unusual pattern
  fSSP[41] = 54; fSSPChOne[41] = 8;
  fSSP[43] = 54; fSSPChOne[43] = 4;
  fSSP[45] = 54; fSSPChOne[45] = 0;
  fSSP[47] = 53; fSSPChOne[47] = 8;
  fSSP[49] = 52; fSSPChOne[49] = 0;
  fSSP[51] = 53; fSSPChOne[51] = 4;
  fSSP[53] = 51; fSSPChOne[53] = 8;
  fSSP[55] = 51; fSSPChOne[55] = 4;
  fSSP[57] = 53; fSSPChOne[57] = 0;
  fSSP[59] = 51; fSSPChOne[59] = 0;
  
  // DSRaS -- "normal" pattern
  fSSP[ 0] = 14; fSSPChOne[ 0] = 0;
  fSSP[ 2] = 13; fSSPChOne[ 2] = 8;
  fSSP[ 4] = 13; fSSPChOne[ 4] = 4;
  fSSP[ 6] = 13; fSSPChOne[ 6] = 0;
  fSSP[ 8] = 12; fSSPChOne[ 8] = 8;
  fSSP[10] = 12; fSSPChOne[10] = 4;
  fSSP[12] = 12; fSSPChOne[12] = 0;
  fSSP[14] = 11; fSSPChOne[14] = 8;
  fSSP[16] = 11; fSSPChOne[16] = 4;
  fSSP[18] = 11; fSSPChOne[18] = 0;

  // MSRaS -- "normal" pattern
  fSSP[20] = 24; fSSPChOne[20] = 0;
  fSSP[22] = 23; fSSPChOne[22] = 8;
  fSSP[24] = 23; fSSPChOne[24] = 4;
  fSSP[26] = 23; fSSPChOne[26] = 0;
  fSSP[28] = 22; fSSPChOne[28] = 8;
  fSSP[30] = 22; fSSPChOne[30] = 4;
  fSSP[32] = 22; fSSPChOne[32] = 0;
  fSSP[34] = 21; fSSPChOne[34] = 8;
  fSSP[36] = 21; fSSPChOne[36] = 4;
  fSSP[38] = 21; fSSPChOne[38] = 0;
  
  // USRaS -- unusual pattern for ARAPUCA
  fSSP[40] = 33; fSSPChOne[40] = 8;
  fSSP[42] = 33; fSSPChOne[42] = 4;
  fSSP[44] = 33; fSSPChOne[44] = 0;
  fSSP[46] = 34; fSSPChOne[46] = 0;
  fSSP[48] = 32; fSSPChOne[48] = 8;
  fSSP[50] = 32; fSSPChOne[50] = 4;
  fSSP[52] = 32; fSSPChOne[52] = 0;
  fSSP[54] = 31; fSSPChOne[54] = 8;
  fSSP[56] = 31; fSSPChOne[56] = 4;
  fSSP[58] = 31; fSSPChOne[58] = 0;


  // The above enables OpDet + HW channel -> OpChannel
  //
  // Fill the maps below to do the reverse by looping through
  // all possible OpDet and HW Channel combinations

  int fMaxOpChannel = 0;
  int fNOpChannels = 0;
  
  for (Index opDet = 0; opDet < 60; opDet++) {
    for (Index hwCh = 0; hwCh < NOpHardwareChannels(opDet); hwCh++) {

      // Find the channel number for this opDet and hw channel
      Index opChannel = OpChannel(opDet, hwCh);

      // Count channels and record the maximum possible channel
      if (opChannel > fMaxOpChannel) fMaxOpChannel = opChannel;
      fNOpChannels++;

      // Fill maps for opChannel -> opDet and hwChannel
      fOpDet[opChannel] = opDet;
      fHWChannel[opChannel] = hwCh;
    }
  }




  //PrintChannelMaps();


  for (Index ionline=0; ionline<fMaxOpChannel; ++ionline)
  {
    if (! IsValidOpChannel(ionline, 60) ) {
      continue;
    }
    
    Index OpDet = OpDetFromOpChannel(ionline);
    Index hwChannel = HardwareChannelFromOpChannel(ionline);
    Index ssp, sspch;
    
    SSPandCh(OpDet, hwChannel, ssp, sspch);

    int APA = ssp/10;

    printf("%5d %5d %5d %5d %5d %5d %5d\n",ionline,APA,ssp,ssp,sspch,OpDet,ionline);
  }

}





//----------------------------------------------------------------------------

Index NOpChannels(Index /*NOpDets*/) {
  return fNOpChannels;
}

//----------------------------------------------------------------------------

Index MaxOpChannel(Index /*NOpDets*/) {
  return fMaxOpChannel;
}

//----------------------------------------------------------------------------

Index NOpHardwareChannels(Index opDet) {

  // ARAPUCAs
  if (opDet == 31 or opDet == 46)
    return 12;
  else
    return 4;  
}

//----------------------------------------------------------------------------

void SSPandCh(Index detNum, Index channel, Index &ssp, Index &sspch) {
  sspch = fSSPChOne.at(detNum) + channel;
  ssp   = fSSP.at(detNum);
    
  // Special handling of ARAPUCA in MSDaS which cross between SSP 62 and 63
  if (sspch > 12) {
    if (ssp != 62) {
      std::cerr << "Invalid address: SSP #" << ssp << ", SSP channel" << sspch << std::endl;;
    }
    ssp += 1;
    sspch -= 12;
  }
  
}

//----------------------------------------------------------------------------

Index OpChannel(Index detNum, Index channel) {
  Index ssp, sspch;
  SSPandCh(detNum, channel, ssp, sspch);

  return OpChannelFromSSP(ssp, sspch);
}

//----------------------------------------------------------------------------

Index OpDetFromOpChannel(Index opChannel) {
  if (!IsValidOpChannel(opChannel, 60)) {
    std::cout << "Requesting an OpDet number for an uninstrumented channel, " << opChannel << std::endl;
    return 99999;
  }
  return fOpDet.at(opChannel);
}

//----------------------------------------------------------------------------

Index HardwareChannelFromOpChannel(Index opChannel) {
  if (!IsValidOpChannel(opChannel, 60)) {
    std::cout << "Requesting an OpDet number for an uninstrumented channel, " << opChannel << std::endl;
    return 99999;
  }
  return fHWChannel.at(opChannel);
}


//----------------------------------------------------------------------------

Index OpChannelFromSSP(Index ssp, Index sspch) {
  // Expects SSP #'s of the from NM where N is APA number and M is SSP within the APA
  // So, IP 504 -> AP # 54

  //         ( (  APA # - 1 )    )*4 + SSP per APA)*12 + SSP channel
  Index ch = ( (trunc(ssp/10) - 1)*4 + ssp%10 - 1 )*12 + sspch;
  return ch;

}

//----------------------------------------------------------------------------

bool  IsValidOpChannel(Index opChannel, Index /*NOpDets*/)
{
  return fOpDet.count(opChannel);
}

//----------------------------------------------------------------------------


void PrintChannelMaps() {


  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << endl << endl;

  std::vector<Index> ssps = { 11, 12, 13, 14,
                              21, 22, 23, 24,
                              31, 32, 33, 34,
                              41, 42, 43, 44,
                              51, 52, 53, 54,
                              61, 62, 63, 64 };
    
  cout << endl << "By SSP" << endl;
  for (Index ssp : ssps) {
    for (Index sspch = 0; sspch < 12; sspch++) {
      cout << setw(2) << ssp << " " << setw(2) << sspch << ": " << setw(3) << OpChannelFromSSP(ssp, sspch) << endl;
    }
  }

  cout << endl << "Beam side" << endl;
  for (Index opDet = 1; opDet < 60; opDet += 2) {
    cout << setw(2) << opDet << ":";
    for (Index hwCh = 0; hwCh < NOpHardwareChannels(opDet); hwCh++) {
      cout << " " << setw(2) << OpChannel(opDet, hwCh);
    }
    cout << endl;
  }


  cout << endl << "Non-Beam side" << endl;
  for (Index opDet = 0; opDet < 60; opDet += 2) {
    cout << setw(2) << opDet << ":";
    for (Index hwCh = 0; hwCh < NOpHardwareChannels(opDet); hwCh++) {
      cout << " " << setw(2) << OpChannel(opDet, hwCh);
    }
    cout << endl;
  }


  cout << endl << "Online -> offline" << endl;
  for (Index opCh = 0; opCh < MaxOpChannel(60); opCh++) {
    cout << setw(3) << opCh << ": ";
    if ( IsValidOpChannel(opCh, 60) ) {
      cout << setw(2) << OpDetFromOpChannel(opCh) << ", "
           << setw(2) << HardwareChannelFromOpChannel(opCh) << endl;
    }
    else {
      cout << "empty channel" << endl;
    }
  }

  cout << endl << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;
  cout << "---------------------------------------------------------------" << endl;

}
