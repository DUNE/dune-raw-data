#include "TString.h"
#include "stdio.h"
#include "TGraph.h"

void checkchannelmap(int itpcset=0, TString filename="protoDUNETPCChannelMap_RCE_v4.txt")
{
  int nchannels=15360;
  int crate[nchannels];
  int slot[nchannels];
  int fiber[nchannels];
  int fembchan[nchannels];
  int streamchan[nchannels];
  int slotid[nchannels];
  int fiberid[nchannels];
  int chipno[nchannels];
  int chipchan[nchannels];
  int asicno[nchannels];
  int asicchannel[nchannels];
  int plane[nchannels];
  int offlinechan[nchannels];

  FILE *infile = fopen(filename,"r");
  for (int i=0; i<nchannels; ++i)
    {
      fscanf(infile,"%d %d %d %d %d %d %d %d %d %d %d %d %d",
	     &crate[i],&slot[i],&fiber[i],&fembchan[i],
	     &streamchan[i],&slotid[i],&fiberid[i],
	     &chipno[i],&chipchan[i],&asicno[i],
	     &asicchannel[i],&plane[i],&offlinechan[i]); 
    }

  float f_ifmb[2560];
  float f_chan[2560];

  for (int i=0;i<2560; ++i)
    {
      int j=2560*itpcset + i;
      int ifmb = 5*fiber[j] + slot[j];
      f_ifmb[i] = ifmb;
      f_chan[i] = offlinechan[j];
    }
  TGraph *mygraph = (TGraph*) new TGraph(2560,f_chan,f_ifmb);
  TString gtitle = "TPC Set ";
  gtitle += itpcset;
  gtitle += ";Offline Channel;IFMB";
  mygraph->SetTitle(gtitle);
  mygraph->SetMarkerColor(4);
  mygraph->Draw("*A");
}
