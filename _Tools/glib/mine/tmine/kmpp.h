/*
 * kmpp.h
 *
 *  Created on: Jan 26, 2011
 *      Author: tadej
 */

#ifndef KMPP_H_
#define KMPP_H_

#include "../../base.h"
#include "../tmine.h"

/**
 * Initialize k-means in a smart way that gives us some theoretical guarantee
 * about clustering quality.
 *
 * See: David Arthur, Sergei Vassilvitskii: k-means++: The advantages of careful seeding,
 * Proceedings of the eighteenth annual ACM-SIAM symposium on Discrete algorithms, 2007
 *
 * We parametrize by vector type and LinAlg package used.
 * If using a custom vector/matrix type that is not available in LinAlg,
 * Make sure it implements the TMatrix interface and LA:AddVec and LA:DotProduct.
 */
template<class V, class LA, class M>
class TKMeans {
protected:
  TCRef CRef;
public:
  friend class TPt<TKMeans<V,LA,M> >;

protected:
	const M *DocVV;
	TVec<TFltV> DCSim;
	int Dim;
	TFullColMatrix Centroids;
	TIntV Assignment;
	int k;
	int maxItr;
	PNotify Notify;
	TRnd Rnd;
    void ChooseSmartCenters(int numLocalTries, TIntV & centers);
    double GetCandidatePotential(const TFltV & sqDistances, int index) const;
    int SelectCentroidCenter(double potential, const TFltV & sqDistances);

    void PutUnitNorm(TFltV & Vec);
    void PutUnitNorm(TIntFltKdV & Vec);
    virtual bool ShouldReassign(double sim, double bestSim, int docIndex) const {
    	return sim > bestSim;
    }
    
    void MakeCentroids(TVec<TIntV> & ClusterDocs, TIntV & CentroidSize);
    TFltV& Centroid(int ClusterId);

public:
    TKMeans(const M *Docs, int K, int maxItr, PNotify Not);
    ~TKMeans() {}
	void Init();
	void Apply();

	int GetDocs() const;

	/** Implement this for row access for custom matrix implementatios */
	/*const V& GetDoc(int DocId) const;*/
		
	int GetK() const;
	int GetDim() const;
	const TFltV& GetCentroid(int ClusterId) const;
	const TIntV& GetAssignment() const;
    double GetClusterCompactness(const TFltV & Cluster) const;
    double GetClusteringCompactness() const;
    double GetClusteringQualityBySize(const TIntV & ClusterSizes) const;
    void GetClustersQualityByDCSim(TFltV& ClusterDCQ) const;
    double GetClusteringQualityByDCSim() const;
    double GetClusteringSSE() const;


};

class TDefaultMatrixAccess {
public:
	static const TIntFltKdV& GetDoc(const TSparseColMatrix *DocVV, int DocId) {
		return DocVV->ColSpVV[DocId];
	}
	static const TFltV& GetDoc(const TFullColMatrix *DocVV, int DocId)  {
		return DocVV->ColV[DocId];
	}
	static const PBowSpV& GetDoc(const TBowMatrix *DocVV, int DocId) {
		return DocVV->ColSpVV[DocId];
	}
};

template class TKMeans<TIntFltKdV, TLinAlg, TSparseColMatrix>;
template class TKMeans<TFltV, TLinAlg, TFullColMatrix>;
template class TKMeans<PBowSpV, TBowLinAlg, TBowMatrix>;

typedef TKMeans<TIntFltKdV, TLinAlg, TSparseColMatrix> TSparseKMeans;
typedef TKMeans<TFltV, TLinAlg, TFullColMatrix> TDenseKMeans;
typedef TKMeans<PBowSpV, TBowLinAlg, TBowMatrix> TBowKMeans;


 
/*
template TSparseKMeans::TSparseKMeans(const TSparseColMatrix *Docs, int K, int maxItr, PNotify Not);
template<TIntFltKdV, TLinAlg, TSparseColMatrix> TSparseKMeans::~TSparseKMeans();
template void TSparseKMeans::Init();
template void TSparseKMeans::Apply();
template int TSparseKMeans::GetDocs() const;
template int TSparseKMeans::GetK() const;
template int TSparseKMeans::GetDim() const;
template const TFltV& TSparseKMeans::GetCentroid(int ClusterId) const;
template const TIntV& TSparseKMeans::GetAssignment() const;
template double TSparseKMeans::GetClusterCompactness(const TFltV & Cluster) const;
template double TSparseKMeans::GetClusteringCompactness() const;
template double TSparseKMeans::GetClusteringQualityBySize(const TIntV & ClusterSizes) const;
template void TSparseKMeans::GetClustersQualityByDCSim(TFltV& ClusterDCQ) const;
template double TSparseKMeans::GetClusteringQualityByDCSim() const;
template double TSparseKMeans::GetClusteringSSE() const;*/

/**
 * Vectors are sparse 64-bit sparse doubles. Also look at TBowKMeans
 */

/**
 * Vectors are dense 64-bit doubles.
 */

/**
 * An interface of TKMeans to the TextGarden tmine package.
 * Should look and act the same as TBowClust.
 */
class TBowKMeansUtils {
public:
	
	/** Compatibility layer */
	static PBowDocPart GetKMeansPartForDocWgtBs(
		const PNotify& Notify,
		const PBowDocWgtBs& Wgt,
		const PBowDocBs& Bow, const PBowSim& BowSim, TRnd& Rnd,
		const int& Clusts, const int& ClustTrials,
		const double& ConvergEps, const int& MnDocsPerClust,
		const TIntFltPrV& DocIdWgtPrV=TIntFltPrV())  {

		TBowMatrix DocMtx(Wgt);
		TBowKMeans KMeans(&DocMtx, Clusts, ClustTrials, Notify);
		KMeans.Init();
		KMeans.Apply();

		PBowDocPart Part = TBowDocPart::New();

		TVec<TIntV> ClusterDIdV(Clusts);
		const TIntV& Assignment = KMeans.GetAssignment();
		for (int i = 0; i < KMeans.GetDocs(); i++) {
			ClusterDIdV[Assignment[i]].Add(i);
		}

		for (int CId = 0; CId < Clusts; CId++) {
			const TFltV& Cluster = KMeans.GetCentroid(CId);
			TStr CNm = "Cluster " + TInt::GetStr(CId);
			double Qual = 0.0;
			PBowSpV ConceptSpV = TBowSpV::New(CId, Cluster, 0.0);
			PBowDocPart SubPart = TBowDocPart::New();
			PBowDocPartClust Clust = TBowDocPartClust::New(Bow, CNm, Qual,
					ClusterDIdV[CId], ConceptSpV, SubPart);
			Part->AddClust(Clust);
		}

		return Part;
	}

};


#include "kmpp.cpp"

#endif /* KMPP_H_ */
