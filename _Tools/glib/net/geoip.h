#ifdef GEOIP_OLD
/////////////////////////////////////////////////
// Geographical-IP
ClassTP(TGeoIpBs, PGeoIpBs)//{
private:
  TStrStrH CountrySNmToLNmH;
  TUIntV CountryMnIpNumV;
  THash<TUInt, TUIntIntPr> MnIpNumToMxIpNumCountryIdPrH;
  THash<TInt, TIntIntFltFltQu> LocIdToCountryIdCityIdLatitudeLongitudeQuH;
  THash<TInt, TInt> LocIdToStateIdH;
  TUIntV LocMnIpNumV;
  THash<TUInt, TUIntIntPr> MnIpNumToMxIpNumLocIdPrH;
  TStrHash<TInt> GeoNmH;
  UndefCopyAssign(TGeoIpBs);
public:
  TGeoIpBs():
    CountrySNmToLNmH(), 
    CountryMnIpNumV(), MnIpNumToMxIpNumCountryIdPrH(), 
    LocIdToCountryIdCityIdLatitudeLongitudeQuH(),
    LocIdToStateIdH(), LocMnIpNumV(), MnIpNumToMxIpNumLocIdPrH(), 
    GeoNmH(){}
  static PGeoIpBs New(){return new TGeoIpBs();}
  ~TGeoIpBs(){}
  TGeoIpBs(TSIn& SIn):
    CountrySNmToLNmH(SIn), 
    CountryMnIpNumV(SIn), MnIpNumToMxIpNumCountryIdPrH(SIn), 
    LocIdToCountryIdCityIdLatitudeLongitudeQuH(SIn),
    LocIdToStateIdH(SIn), LocMnIpNumV(SIn), 
    MnIpNumToMxIpNumLocIdPrH(SIn), GeoNmH(SIn){}
  static PGeoIpBs Load(TSIn& SIn){return new TGeoIpBs(SIn);}
  void Save(TSOut& SOut){
    CountrySNmToLNmH.Save(SOut);
    CountryMnIpNumV.Save(SOut); MnIpNumToMxIpNumCountryIdPrH.Save(SOut); 
    LocIdToCountryIdCityIdLatitudeLongitudeQuH.Save(SOut);
    LocIdToStateIdH.Save(SOut); LocMnIpNumV.Save(SOut); 
    MnIpNumToMxIpNumLocIdPrH.Save(SOut); GeoNmH.Save(SOut);}

  // search
  int GetCountryNm(const TStr& IpNum, TStr& CountrySNm, TStr& CountryLNm);
  int GetCountryNm(const uint& IpNum, TStr& CountrySNm, TStr& CountryLNm);
  int GetLocation(const TStr& IpNum, TStr& CountrySNm, 
    TStr& CityNm, double& Latitude, double& Longitude);  
  int GetLocation(const uint& IpNum, TStr& CountrySNm, 
    TStr& CityNm, double& Latitude, double& Longitude);

  // state
  bool IsState(const int& LocId) const {
    return LocIdToStateIdH.IsKey(LocId); }
  TStr GetStateNm(const int& LocId) const { 
    return GetGeoNm(LocIdToStateIdH.GetDat(LocId)); }

  // Ids
  TStr GetGeoNm(const int& GeoId) const {return GeoNmH.GetKey(GeoId);}
  int GetGeoId(const TStr& GeoNm) const {return GeoNmH.GetKeyId(GeoNm);}

  // files
  static PGeoIpBs LoadCsv(const TStr& GeoIpFPath, const bool& CountriesOnlyP=false);
  static PGeoIpBs LoadBin(const TStr& FNm){
    TFIn SIn(FNm); return Load(SIn);}
  void SaveBin(const TStr& FNm){
    TFOut SOut(FNm); Save(SOut);}
};

#else

/////////////////////////////////////////////////
// Geographical-IP-Location-Descriptor
class TGeoIpLocDesc{
public:
  TStr CountryNm;
  TStr RegionNm;
  TStr CityNm;
  TStr PostalCode;
  TFlt Latitude;
  TFlt Longitude;
  TStr MetroCode;
  TStr AreaCode;
public:
  TGeoIpLocDesc(){}
  TGeoIpLocDesc(TSIn& SIn):
    CountryNm(SIn), RegionNm(SIn), CityNm(SIn), PostalCode(SIn), 
    Latitude(SIn), Longitude(SIn), MetroCode(SIn), AreaCode(SIn){}
  void Save(TSOut& SOut) const {
    CountryNm.Save(SOut); RegionNm.Save(SOut); CityNm.Save(SOut); 
    PostalCode.Save(SOut); Latitude.Save(SOut); Longitude.Save(SOut); 
    MetroCode.Save(SOut); AreaCode.Save(SOut);}

  TGeoIpLocDesc& operator=(const TGeoIpLocDesc& LocDesc){
    if (this!=&LocDesc){
      CountryNm=LocDesc.CountryNm; RegionNm=LocDesc.RegionNm; CityNm=LocDesc.CityNm;
      PostalCode=LocDesc.PostalCode; Latitude=LocDesc.Latitude; Longitude=LocDesc.Longitude;
      MetroCode=LocDesc.MetroCode; AreaCode=LocDesc.AreaCode;}
    return *this;}
};

/////////////////////////////////////////////////
// Geographical-IP-Organization-Descriptor
class TGeoIpOrgDesc{
public:
  TUInt MxIpNum;
  TInt IspNmId;
  TInt OrgNmId;
  TInt LocId;
public:
  TGeoIpOrgDesc(){}
  TGeoIpOrgDesc(TSIn& SIn):
    MxIpNum(SIn), IspNmId(SIn), OrgNmId(SIn), LocId(SIn){}
  void Save(TSOut& SOut) const {
    MxIpNum.Save(SOut); IspNmId.Save(SOut); OrgNmId.Save(SOut); LocId.Save(SOut);}

  TGeoIpOrgDesc& operator=(const TGeoIpOrgDesc& OrgDesc){
    if (this!=&OrgDesc){
      MxIpNum=OrgDesc.MxIpNum; IspNmId=OrgDesc.IspNmId; OrgNmId=OrgDesc.OrgNmId; LocId=OrgDesc.LocId;}
    return *this;}
};

/////////////////////////////////////////////////
// Geographical-IP
ClassTP(TGeoIpBs, PGeoIpBs)//{
private:
  TStrStrH CountrySNmToLNmH;
  THash<TInt, TGeoIpLocDesc> LocIdToLocDescH;
  TUIntV OrgMnIpNumV;
  THash<TUInt, TGeoIpOrgDesc> MnIpNumToOrgDescH;
  TStrHash<TInt> StrH;
  UndefCopyAssign(TGeoIpBs);
public:
  TGeoIpBs():
    CountrySNmToLNmH(), LocIdToLocDescH(), 
    OrgMnIpNumV(), MnIpNumToOrgDescH(), StrH(){}
  static PGeoIpBs New(){return new TGeoIpBs();}
  ~TGeoIpBs(){}
  TGeoIpBs(TSIn& SIn):
    CountrySNmToLNmH(SIn), LocIdToLocDescH(SIn), 
    OrgMnIpNumV(SIn), MnIpNumToOrgDescH(SIn), StrH(SIn){}
  static PGeoIpBs Load(TSIn& SIn){return new TGeoIpBs(SIn);}
  void Save(TSOut& SOut){
    CountrySNmToLNmH.Save(SOut); LocIdToLocDescH.Save(SOut);
    OrgMnIpNumV.Save(SOut); MnIpNumToOrgDescH.Save(SOut); StrH.Save(SOut);}

  // organization search
  int GetOrgId(const TStr& IpNumStr);
  int GetOrgId(const uint& IpNum);

  // organization context
  TStr GetOrgNm(const int& OrgId) const {return StrH.GetKey(MnIpNumToOrgDescH[OrgId].OrgNmId);}
  TStr GetIspNm(const int& OrgId) const {return StrH.GetKey(MnIpNumToOrgDescH[OrgId].IspNmId);}
  TStr GetCountryNm(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).CountryNm;}
  TStr GetRegionNm(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).RegionNm;}
  TStr GetCityNm(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).CityNm;}
  TStr GetPostalCode(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).PostalCode;}
  TFlt GetLatitude(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).Latitude;}
  TFlt GetLongitude(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).Longitude;}
  TStr GetMetroCode(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).MetroCode;}
  TStr GetAreaCode(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).AreaCode;}

  static void WrOrgInfo(const PGeoIpBs& GeoIpBs, const TStr& IpNumStr);

  // files
  static PGeoIpBs LoadCsv(const TStr& GeoIpFPath);
  static PGeoIpBs LoadBin(const TStr& FNm){
    TFIn SIn(FNm); return Load(SIn);}
  void SaveBin(const TStr& FNm){
    TFOut SOut(FNm); Save(SOut);}
};

#endif