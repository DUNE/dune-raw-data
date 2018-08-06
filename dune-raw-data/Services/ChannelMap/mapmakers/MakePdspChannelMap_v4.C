
/*************************************************************************
    > File Name: makeMap.cc
    > Author: Jingbo Wang
    > Mail: jiowang@ucdavis.edu 
    > Created Time: Tue Jan 23 13:57:14 2018
 ************************************************************************/

using namespace std;

// Implementation of Table 5 in DocDB 4064: FEMB channel (chipNo & chipChannel) to plane type
int plane[8][16] = {
										 {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2},
										 {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2}, 
										 {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
										 {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
										 {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2},
										 {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2},
										 {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
										 {2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0}
	                 };

// Implementation of Table 5 in DocDB 4064: FEMB channel (chipNo & chipChannel) to plane channel number                 
int planeCh[8][16] = {
										 {19, 17, 15, 13, 11, 19, 17, 15, 13, 11, 23, 21, 19, 17, 15, 13},
										 {9,  7,  5,  3,  1,  9,  7,  5,  3,  1,  11, 9,  7,  5,  3,  1}, 
										 {14, 16, 18, 20, 22, 24, 12, 14, 16, 18, 20, 12, 14, 16, 18, 20},
										 {2,  4,  6,  8,  10, 12, 2,  4,  6,  8,  10, 2,  4,  6,  8,  10},
										 {29, 27, 25, 23, 21, 29, 27, 25, 23, 21, 35, 33, 31, 29, 27, 25},
										 {39, 37, 35, 33, 31, 39, 37, 35, 33, 31, 47, 45, 43, 41, 39, 37},
										 {26, 28, 30, 32, 34, 36, 22, 24, 26, 28, 30, 22, 24, 26, 28, 30},
										 {38, 40 ,42, 44, 46, 48, 32, 34, 36, 38, 40, 32, 34, 36, 38, 40}
	                 };

// Implementation of SLAC's channel map: FEMB channel (chipNo & chipChannel) to RCE channel number                 
int rceCh[8][16] = {
										 {14, 12, 10, 8,  6,  4,  2,  0,  30, 28, 26, 24, 22, 20, 18, 16},
										 {46, 44, 42, 40, 38, 36, 34, 32, 62, 60, 58, 56, 54, 52, 50, 48},
										 {15, 13, 11, 9,  7,  5,  3,  1,  31, 29, 27, 25, 23, 21, 19, 17},
										 {47, 45, 43, 41, 39, 37, 35, 33, 63, 61, 59, 57, 55, 53, 51, 49},
										 {78, 76, 74, 72, 70, 68, 66, 64, 94, 92, 90, 88, 86, 84, 82, 80},
										 {110, 108, 106, 104, 102, 100, 98, 96, 126, 124, 122, 120, 118, 116, 114, 112},
										 {79,  77,  75,  73,  71,  69,  67, 65, 95,  93,  91,  89,  87,  85,  83,  81},
										 {111, 109, 107, 105, 103, 101, 99, 97, 127, 125, 123, 121, 119, 117, 115, 113}
	                 };

// Implementation of the FELIX channel map: FEMB channel (chipNo & chipChannel) to FELIX fragment channel index.  The second 128 channels in the frame
// have the same behavior but are offset by 128

int felixCh[8][16] = {
                                                                                 {7, 6, 5, 4, 3, 2, 1, 0, 23, 22, 21, 20, 19, 18, 17, 16},
                                                                                 {39, 38, 37, 36, 35, 34, 33, 32, 55, 54, 53, 52, 51, 50, 49, 48},
                                                                                 {15, 14, 13, 12, 11, 10, 9, 8, 31, 30, 29, 28, 27, 26, 25, 24},
                                                                                 {47, 46, 45, 44, 43, 42, 41, 40, 63, 62, 61, 60, 59, 58, 57, 56},
                                                                                 {71, 70, 69, 68, 67, 66, 65, 64, 87, 86, 85, 84, 83, 82, 81, 80},
                                                                                 {103, 102, 101, 100, 99, 98, 97, 96, 119, 118, 117, 116, 115, 114, 113, 112},
                                                                                 {79, 78, 77, 76, 75, 74, 73, 72, 95, 94, 93, 92, 91, 90, 89, 88},
                                                                                 {111, 110, 109, 108, 107, 106, 105, 104, 127, 126, 125, 124, 123, 122, 121, 120}
};


  // indexed by offline-oriented slot and fiber, to give online slot and fiber
  // implements Table 4 and Fig. 3a in DocDB 4064. 
// offline slot and fiber numbers increase counterclockwise around the even APA's

  // Map for offline APA 0 (== online APA 3), and other even offline APA's

int fibermap[4] = {2, 3, 0, 1};

void MakePdspChannelMap_v4() {
 
  ofstream fmaprce;
  fmaprce.open ("protoDUNETPCChannelMap_RCE_v4.txt");
  ofstream fmapfelix;
  fmapfelix.open ("protoDUNETPCChannelMap_FELIX_v4.txt");
  int nCrate = 6;
  int nSlotPerCrate = 5;
  int nFiberPerSlot = 4;
  int nChipPerFEMB = 8;
  int nASICPerChip = 2;
  int nChannelPerASIC = 8;
  int FEMBChannel = 0;
  int RCEStreamChannel = 0;
  int FELIXStreamChannel = 0;
  int chipChannel = 0;
  int asicChannel = 0;
  int slotID = 0;
  int fiberID = 0;
  
  int Asic = 0;
  int offlineChannel  = 0;

  for(int crateNo = 0; crateNo<nCrate; crateNo++) {
    bool APAisLeft =  ( crateNo % 2 == 1 );   // if false, then APA is on the right-hand side of the beam

    int planeType = 0;
    vector<int> Uwire, Vwire, Zwire;
    for(int slotNo = 0;slotNo<nSlotPerCrate;slotNo++) {
      for(int fiberNo = 0;fiberNo<nFiberPerSlot;fiberNo++) {
	FEMBChannel = 0;
	RCEStreamChannel = 0;
	FELIXStreamChannel = 0;
	for(int chipNo = 0;chipNo<nChipPerFEMB;chipNo++) {  
	  chipChannel =0; 	 	 	 
	  for(int asicNo = 0;asicNo<nASICPerChip;asicNo++) {
	    asicChannel = 0;
	    for(int chNo = 0; chNo<nChannelPerASIC; chNo++) {
	      //plane type
	      planeType =  plane[chipNo][chipChannel];

	      RCEStreamChannel = rceCh[chipNo][chipChannel]; 
	      FELIXStreamChannel = felixCh[chipNo][chipChannel];  

	      int slotNoOnline = slotNo;
	      int fiberNoOnline = fibermap[fiberNo];

     	 	            	 	        	
	      //offline number
	      if(planeType == 0) {
		Uwire.push_back(planeCh[chipNo][chipChannel]);
		// old calc -- need to invert the order of U-plane channels
		//offlineChannel = 2560*crateNo + 40*(4*slotNo + fiberNo)+ (planeCh[chipNo][chipChannel]-1);

		int itmp = 40*(slotNo + 5*fiberNo)+ (planeCh[chipNo][chipChannel]-1);  // should go from 0 to 799

		if (! APAisLeft)
		  {
		    if (itmp < 400)
		      {
			itmp = 399-itmp;
		      }
		    else
		      {
		        itmp = 799 + 400 - itmp;
		      }
		  }
		else
		  {
	             itmp = 799 - itmp;
		  }
		offlineChannel = 2560*crateNo + itmp;
	      }
	      if(planeType == 1) {
		Vwire.push_back(planeCh[chipNo][chipChannel]);
		//offlineChannel = 2560*crateNo +  800 + 40*(4*slotNo + fiberNo) + (planeCh[chipNo][chipChannel]-1);

		int itmp = 40*(slotNo + 5*fiberNo) + (planeCh[chipNo][chipChannel]-1);  // goes from 0 to 799
		if (APAisLeft) 
		  {
		    if (itmp < 400)
		      {
  		        itmp = itmp + 400;
		      }
		    else
		      {
			itmp = itmp - 400;
		      }
		  }
		else
		  {
		    // nothing needs to be done here.
		  }
		offlineChannel = 2560*crateNo +  800 + itmp;

	      }
	      if(planeType == 2) {
		Zwire.push_back(planeCh[chipNo][chipChannel]);   
		//offlineChannel = 2560*crateNo + 1600 + 48*(4*slotNo + fiberNo)  + (planeCh[chipNo][chipChannel]-1);

		int itmp = 48*(slotNo + 5*fiberNo)  + (planeCh[chipNo][chipChannel]-1);  // goes from 0 to 959
		// the Z2 channels increase in the opposite order

		if (! APAisLeft) 
		  {
		    if (itmp<480)
		      {
			// do nothing
		      }
		    else
		      {
			itmp = 480 + 959 - itmp;
		      }
		  }
		else
		  {
		    if (itmp<480)
		      {
			itmp = 959 - itmp;
		      }
		    else
		      {
			itmp = itmp - 480;
		      }
		  }

                offlineChannel = 2560*crateNo + 1600 + itmp;
	      }
     	 	       
	      fmaprce<<crateNo<<"\t"<<slotNoOnline<<"\t"<<fiberNoOnline<<"\t"<<FEMBChannel
		     <<"\t"<<RCEStreamChannel<<"\t"<<slotID<<"\t"<<fiberID
		     <<"\t"<<chipNo<<"\t"<<chipChannel<<"\t"<<asicNo<<"\t"
		     <<asicChannel<<"\t"<<planeType<<"\t"<<offlineChannel<<endl;  
	      fmapfelix<<crateNo<<"\t"<<slotNoOnline<<"\t"<<fiberNoOnline<<"\t"<<FEMBChannel
		       <<"\t"<<FELIXStreamChannel<<"\t"<<slotID<<"\t"<<fiberID
		       <<"\t"<<chipNo<<"\t"<<chipChannel<<"\t"<<asicNo<<"\t"
		       <<asicChannel<<"\t"<<planeType<<"\t"<<offlineChannel<<endl;  
	      FEMBChannel++;
	      chipChannel++;
	      asicChannel++;
	      //     	 	       offlineChannel++; 	 	
	    }
	  }    	 	   
	} 
	fiberID++; 
      }
      slotID++;
    }
  }
  fmaprce.close();
  fmapfelix.close();
  
}
