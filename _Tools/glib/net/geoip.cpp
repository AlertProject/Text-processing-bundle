#ifdef GEOIP_OLD
/////////////////////////////////////////////////
// Geographical-IP
int TGeoIpBs::GetCountryNm(const TStr& IpNumStr, TStr& CountrySNm, TStr& CountryLNm){
  // split ip-num to sub-number-strings
  TStrV IpSubNumStrV; IpNumStr.SplitOnAllCh('.', IpSubNumStrV, false);
  // convert sub-number-strings to sub-numbers and ip-number
  int IpSubNum0, IpSubNum1, IpSubNum2, IpSubNum3;
  uint IpNum;
  if (
   IpSubNumStrV[0].IsInt(true, 0, 255, IpSubNum0)&&
   IpSubNumStrV[1].IsInt(true, 0, 255, IpSubNum1)&&
   IpSubNumStrV[2].IsInt(true, 0, 255, IpSubNum2)&&
   IpSubNumStrV[3].IsInt(true, 0, 255, IpSubNum3)){
    IpNum=16777216*IpSubNum0+65536*IpSubNum1+256*IpSubNum2+IpSubNum3;
  } else {
    return -1;
  }
  return GetCountryNm(IpNum, CountrySNm, CountryLNm);
}

int TGeoIpBs::GetCountryNm(const uint& IpNum, TStr& CountrySNm, TStr& CountryLNm){
  // prepare country-names
  CountrySNm="--"; CountryLNm="Unknown";
  // get country-id from ip-number
  int CountryId=-1;
  int IpNumN; CountryMnIpNumV.SearchBin(IpNum+1, IpNumN);
  if (IpNumN>0){
    uint MnIpNum=CountryMnIpNumV[IpNumN-1];
    uint MxIpNum=MnIpNumToMxIpNumCountryIdPrH.GetDat(MnIpNum).Val1;
    if ((MnIpNum<=IpNum)&&(IpNum<=MxIpNum)){
      CountryId=MnIpNumToMxIpNumCountryIdPrH.GetDat(MnIpNum).Val2;
    }
  }
  // get country-names
  if (CountryId!=-1){
    CountrySNm=CountrySNmToLNmH.GetKey(CountryId);
    CountryLNm=CountrySNmToLNmH[CountryId];
  }
  return CountryId;
}

int TGeoIpBs::GetLocation(const TStr& IpNumStr, 
 TStr& CountrySNm, TStr& CityNm, double& Latitude, double& Longitude){
  // split ip-num to sub-number-strings
  TStrV IpSubNumStrV; IpNumStr.SplitOnAllCh('.', IpSubNumStrV, false);
  // convert sub-number-strings to sub-numbers and ip-number
  int IpSubNum0, IpSubNum1, IpSubNum2, IpSubNum3;
  uint IpNum;
  if (
   IpSubNumStrV[0].IsInt(true, 0, 255, IpSubNum0)&&
   IpSubNumStrV[1].IsInt(true, 0, 255, IpSubNum1)&&
   IpSubNumStrV[2].IsInt(true, 0, 255, IpSubNum2)&&
   IpSubNumStrV[3].IsInt(true, 0, 255, IpSubNum3)){
    IpNum=16777216*IpSubNum0+65536*IpSubNum1+256*IpSubNum2+IpSubNum3;
  } else {
    return -1;
  }
  return GetLocation(IpNum, CountrySNm, CityNm, Latitude, Longitude);
}

int TGeoIpBs::GetLocation(const uint& IpNum, 
 TStr& CountrySNm, TStr& CityNm, double& Latitude, double& Longitude){
  // prepare unknown location info
  CountrySNm="--"; CityNm="Unknown"; Latitude=0; Longitude=0;

  // get location-id from ip-number
  int LocId=-1;
  int IpNumN; LocMnIpNumV.SearchBin(IpNum+1, IpNumN);
  if (IpNumN>0){
    uint MnIpNum=LocMnIpNumV[IpNumN-1];
    uint MxIpNum=MnIpNumToMxIpNumLocIdPrH.GetDat(MnIpNum).Val1;
    if ((MnIpNum<=IpNum)&&(IpNum<=MxIpNum)){
      LocId=MnIpNumToMxIpNumLocIdPrH.GetDat(MnIpNum).Val2;
    }
  }
  // get location info
  if (LocId!=-1){
    int CountrySNmId=LocIdToCountryIdCityIdLatitudeLongitudeQuH.GetDat(LocId).Val1;
    if (CountrySNmId!=-1){CountrySNm=GeoNmH.GetKey(CountrySNmId);}
    int CityNmId=LocIdToCountryIdCityIdLatitudeLongitudeQuH.GetDat(LocId).Val2;
    if (CityNmId!=-1){CityNm=GeoNmH.GetKey(CityNmId);}
    Latitude=LocIdToCountryIdCityIdLatitudeLongitudeQuH.GetDat(LocId).Val3;
    Longitude=LocIdToCountryIdCityIdLatitudeLongitudeQuH.GetDat(LocId).Val4;
  }
  return LocId;
}

PGeoIpBs TGeoIpBs::LoadCsv(const TStr& GeoIpFPath, const bool& CountriesOnlyP){
  PGeoIpBs GeoIpBs=TGeoIpBs::New();
  // filenames
  TStr GeoIpNrFPath=TStr::GetNrFPath(GeoIpFPath);
  TStr CountryFNm=GeoIpNrFPath+"GeoIPCountryWhois.csv";
  TStr CityLocationFNm=GeoIpNrFPath+"GeoLiteCity-Location.csv";
  TStr CityBlocksFNm=GeoIpNrFPath+"GeoLiteCity-Blocks.csv";
  // country-level data
  {PSs Ss=TSs::LoadTxt(ssfCommaSep, CountryFNm, TNotify::StdNotify);
  for (int Y=0; Y<Ss->GetYLen(); Y++){
    uint MnIpNum=Ss->At(2, Y).GetUInt();
    uint MxIpNum=Ss->At(3, Y).GetUInt();;
    TStr CountrySNm=Ss->At(4, Y);
    TStr CountryLNm=Ss->At(5, Y);
    GeoIpBs->CountrySNmToLNmH.AddDat(CountrySNm)=CountryLNm;
    int CountryId=GeoIpBs->CountrySNmToLNmH.GetKeyId(CountrySNm);
    GeoIpBs->MnIpNumToMxIpNumCountryIdPrH.AddDat(MnIpNum, TUIntIntPr(MxIpNum, CountryId));
    GeoIpBs->CountryMnIpNumV.Add(MnIpNum);
  }
  printf("Sorting ... ");
  GeoIpBs->CountryMnIpNumV.Sort();}
  printf("Done.\n");
  if (!CountriesOnlyP){
    const int UsCountryId = GeoIpBs->GeoNmH.AddKey("US");
    // city-locations data
    {PSs Ss=TSs::LoadTxt(ssfCommaSep, CityLocationFNm, TNotify::StdNotify, false);
    for (int Y=2; Y<Ss->GetYLen(); Y++){
      int LocId=Ss->At(0, Y).GetInt();
      TStr CountrySNm=Ss->At(1, Y);
      int CountryId=GeoIpBs->GeoNmH.AddKey(CountrySNm);
      TStr CityNm=Ss->At(3, Y);
      int CityId=GeoIpBs->GeoNmH.AddKey(CityNm);
      double Latitude=Ss->At(5, Y).GetFlt();
      double Longitude=Ss->At(6, Y).GetFlt();
      // add data
      GeoIpBs->LocIdToCountryIdCityIdLatitudeLongitudeQuH.AddDat(
       LocId, TIntIntFltFltQu(CountryId, CityId, Latitude, Longitude));
      // for us we also get state
	    if (CountryId==UsCountryId) {
        TStr StateNm=Ss->At(2, Y);
        int StateId=GeoIpBs->GeoNmH.AddKey(StateNm);
		    GeoIpBs->LocIdToStateIdH.AddDat(LocId, StateId);
	    }
    }}
    // city-blocks data
    {PSs Ss=TSs::LoadTxt(ssfCommaSep, CityBlocksFNm, TNotify::StdNotify, false);
    for (int Y=2; Y<Ss->GetYLen(); Y++){
      uint MnIpNum=Ss->At(0, Y).GetUInt();
      uint MxIpNum=Ss->At(1, Y).GetUInt();
      int LocId=Ss->At(2, Y).GetInt();
      GeoIpBs->MnIpNumToMxIpNumLocIdPrH.AddDat(MnIpNum, TUIntIntPr(MxIpNum, LocId));
      GeoIpBs->LocMnIpNumV.Add(MnIpNum);
    }
    printf("Sorting ... ");
    GeoIpBs->LocMnIpNumV.Sort();}
    printf("Done.\n");
  }
  // return geoip base
  return GeoIpBs;
}

#else 

/////////////////////////////////////////////////
// Organization-Geographical-IP
int TGeoIpBs::GetOrgId(const TStr& IpNumStr){
  uint IpNum;
  if (TUInt::IsIpStr(IpNumStr, IpNum)) {    
    return GetOrgId(IpNum);
  } else {
    return -1;
  }
}

int TGeoIpBs::GetOrgId(const uint& IpNum){
  // get location-id from ip-number
  int OrgId=-1;
  int IpNumN; OrgMnIpNumV.SearchBin(IpNum+1, IpNumN);
  if (IpNumN>0){
    uint MnIpNum=OrgMnIpNumV[IpNumN-1];
    uint MxIpNum=MnIpNumToOrgDescH.GetDat(MnIpNum).MxIpNum;
    if ((MnIpNum<=IpNum)&&(IpNum<=MxIpNum)){
      OrgId=MnIpNumToOrgDescH.GetKeyId(MnIpNum);
    }
  }
  return OrgId;
}

void TGeoIpBs::WrOrgInfo(const PGeoIpBs& GeoIpBs, const TStr& IpNumStr){
  int OrgId=GeoIpBs->GetOrgId(IpNumStr);
  if (OrgId==-1){
    printf("Wrong IP\n");
  } else {
    printf("Org: %s\n", GeoIpBs->GetOrgNm(OrgId).CStr());
    printf("ISP: %s\n", GeoIpBs->GetIspNm(OrgId).CStr());
    printf("Country: %s\n", GeoIpBs->GetCountryNm(OrgId).CStr());
    printf("Region: %s\n", GeoIpBs->GetRegionNm(OrgId).CStr());
    printf("City: %s\n", GeoIpBs->GetCityNm(OrgId).CStr());
    printf("Postal: %s\n", GeoIpBs->GetPostalCode(OrgId).CStr());
    printf("Latitude: %f\n", GeoIpBs->GetLatitude(OrgId));
    printf("Longitude: %f\n", GeoIpBs->GetLongitude(OrgId));
    printf("Metro: %s\n", GeoIpBs->GetMetroCode(OrgId).CStr());
    printf("Area: %s\n", GeoIpBs->GetAreaCode(OrgId).CStr());
  }
}

PGeoIpBs TGeoIpBs::LoadCsv(const TStr& GeoIpFPath){
  PGeoIpBs GeoIpBs=TGeoIpBs::New();
  // filenames
  TStr GeoIpNrFPath=TStr::GetNrFPath(GeoIpFPath);
  TStr LocFNm=GeoIpNrFPath+"GeoIPCity-144-Location.csv";
  TStr OrgFNm=GeoIpNrFPath+"GeoIPCityISPOrg-144.csv";

  // location data
  {PSs Ss=TSs::LoadTxt(ssfCommaSep, LocFNm, TNotify::StdNotify, false);
  printf("Load %s ...\n", LocFNm.CStr());
  for (int Y=2; Y<Ss->GetYLen(); Y++){
    if (Y%1000==0){printf("%d\r", Y);}
    int LocId=Ss->At(0, Y).GetInt();
    TGeoIpLocDesc& LocDesc=GeoIpBs->LocIdToLocDescH.AddDat(LocId);
    LocDesc.CountryNm=Ss->At(1, Y);
    LocDesc.RegionNm=Ss->At(2, Y);
    LocDesc.CityNm=Ss->At(3, Y);
    LocDesc.PostalCode=Ss->At(4, Y);
    LocDesc.Latitude=Ss->At(5, Y).GetFlt();
    LocDesc.Longitude=Ss->At(6, Y).GetFlt();
    LocDesc.MetroCode=Ss->At(7, Y);
    LocDesc.AreaCode=Ss->At(8, Y);
  }
  printf("\nDone.\n");}

  // organization data
  {PSs Ss=TSs::LoadTxt(ssfCommaSep, OrgFNm, TNotify::StdNotify, false);
  printf("Load %s ...\n", OrgFNm.CStr());
  for (int Y=2; Y<Ss->GetYLen(); Y++){
    if (Y%1000==0){printf("%d\r", Y);}
    TUInt MnIpNum=Ss->At(0, Y).GetUInt();
    TGeoIpOrgDesc& OrgDesc=GeoIpBs->MnIpNumToOrgDescH.AddDat(MnIpNum);
    OrgDesc.MxIpNum=Ss->At(1, Y).GetUInt();
    OrgDesc.LocId=Ss->At(2, Y).GetInt();
    TStr IspNm=Ss->At(3, Y);
    OrgDesc.IspNmId=GeoIpBs->StrH.AddKey(IspNm);
    TStr OrgNm=Ss->At(4, Y);
    OrgDesc.OrgNmId=GeoIpBs->StrH.AddKey(OrgNm);
    GeoIpBs->OrgMnIpNumV.Add(MnIpNum);
  }
  printf("\nDone.\n");}
  printf("Sorting IPs... ");
  GeoIpBs->OrgMnIpNumV.Sort();
  printf("Done.\n");

  // return geoip base
  return GeoIpBs;
}

#endif