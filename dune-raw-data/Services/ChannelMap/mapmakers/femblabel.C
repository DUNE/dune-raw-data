
{

  int fvInstalledAPA[6];
  int fvTPCSet_VsInstalledAPA[6];

  // APA numbering -- hardcoded here.
  // Installation numbering:
  //             APA5  APA6  APA4
  //  beam -->
  //             APA3  APA2  APA1
  //
  //  The Offline numbering:
  //             APA1  APA3  APA5
  //  beam -->
  //             APA0  APA2  APA4
  //
  fvInstalledAPA[0] = 3;
  fvInstalledAPA[1] = 5;
  fvInstalledAPA[2] = 2;
  fvInstalledAPA[3] = 6;
  fvInstalledAPA[4] = 1;
  fvInstalledAPA[5] = 4;

  // and the inverse map -- shifted by 1 -- the above list must start counting at 1.

  for (size_t i=0; i<6; ++i)
    {
      fvTPCSet_VsInstalledAPA[fvInstalledAPA[i]-1] = i;
    }

  int fibermapoffsetright[4] = {3, 2, 1, 0};  
  int fibermapoffsetleft[4] = {0, 1, 2, 3};  

  int nCrate = 6;
  int nSlotPerCrate = 5;
  int nFiberPerSlot = 4;
  int fiberID = 0;

  for(int crateNo = 0; crateNo<nCrate; crateNo++) {
    bool APAisLeft =  ( crateNo % 2 == 1 );   // if false, then APA is on the right-hand side of the beam

    for(int slotNo = 0;slotNo<nSlotPerCrate;slotNo++) {
      for(int fiberNo = 0;fiberNo<nFiberPerSlot;fiberNo++) {

	int femblabel = 100*fvInstalledAPA[crateNo] + nSlotPerCrate*(fibermapoffsetright[fiberNo]) + (nSlotPerCrate - slotNo);
	if (APAisLeft)
	  {
	    femblabel = 100*fvInstalledAPA[crateNo] + nSlotPerCrate*(fibermapoffsetleft[fiberNo]) + (nSlotPerCrate - slotNo);
	  }
	std::cout << fiberID << " " << femblabel << " " << slotNo+1 << std::endl;
	fiberID++;   // these are the global fiber ID's in the channel map
      }
    }
  }
}
