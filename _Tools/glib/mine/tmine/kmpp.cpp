#ifndef KMPP_CPP_
#define KMPP_CPP_
/*
 * kmpp.cpp
 *
 *  Created on: Jan 26, 2011
 *      Author: tadej
 *
 *
 */
#include "kmpp.h"

template<class V, class LA, class M>
TKMeans<V, LA, M>::TKMeans(const M *Docs, int K, int MaxItr, PNotify Not) :
		DocVV(Docs), DCSim(K), k(K), maxItr(MaxItr), Notify(Not), Rnd(0) {
	Centroids.ColV.Gen(k);
	Centroids.ColN = k;
	Centroids.RowN = DocVV->GetRows();
	Assignment.Gen(GetDocs());
	for (int CId = 0; CId < k; CId++) {
		DCSim[CId].Gen(GetDocs());
	}
}

template<class V, class LA, class M>
void TKMeans<V, LA, M>::Init() {

	// Initially, no nodes belong to any cluster
	Assignment.PutAll(-1);

	TIntV Centers;
	// Select good centers for clusters
	ChooseSmartCenters(3 + (int) log((double) k), Centers);
	for (int i = 0; i < Centers.Len(); i++) {
		Assignment[Centers[i]] = i;
	}

	for (int i = 0; i < k; i++) {
		Centroids.ColV[i].Gen(GetDim());
	}

	TVec<TIntV> ClusterDocs(k);
	for (int d = 0; d < GetDocs(); d++) {
		if (Assignment[d] != -1) {
			ClusterDocs[Assignment[d]].Add(d);
		}
	}

	TIntV CentroidSize(k);
	MakeCentroids(ClusterDocs, CentroidSize);

	for (int c = 0; c < k; c++) {
		PutUnitNorm(Centroid(c));
	}
}

template<class V, class LA, class M>
int TKMeans<V, LA, M>::GetDocs() const {
	return DocVV->GetCols();
}

template<class V, class LA, class M>
int TKMeans<V, LA, M>::GetDim() const {
	return DocVV->GetRows();
}

template<class V, class LA, class M>
int TKMeans<V, LA, M>::GetK() const {
	return k;
}

template<class V, class LA, class M>
const TFltV& TKMeans<V, LA, M>::GetCentroid(int ClusterId) const {
	return Centroids.ColV[ClusterId];
}

template<class V, class LA, class M>
double TKMeans<V, LA, M>::GetClusterCompactness(const TFltV& Cluster) const {
	return TLinAlg::DotProduct(Cluster, Cluster);
}

template<class V, class LA, class M>
double TKMeans<V, LA, M>::GetClusteringCompactness() const {
	double q = 0.0;
	for (int i = 0; i < k; i++) {
		q += GetClusterCompactness(GetCentroid(i));
	}
	return q;
}

template<class V, class LA, class M>
double TKMeans<V, LA, M>::GetClusteringQualityBySize(
		const TIntV& ClusterSizes) const {
	double q = 0.0;
	for (int i = 0; i < k; i++) {
		q += GetClusterCompactness(GetCentroid(i)) * ClusterSizes[i];
	}
	return q / GetDocs();
}

template<class V, class LA, class M>
void TKMeans<V, LA, M>::GetClustersQualityByDCSim(TFltV& ClusterDCQ) const {
	for (int DocId = 0; DocId < Assignment.Len(); DocId++) {
		double Sim = DCSim[Assignment[DocId]][DocId];
		ClusterDCQ[Assignment[DocId]] += Sim;
	}
}

template<class V, class LA, class M>
double TKMeans<V, LA, M>::GetClusteringQualityByDCSim() const {
	TFltV ClusterDCQ(k);
	GetClustersQualityByDCSim(ClusterDCQ);
	return TLinAlg::SumVec(ClusterDCQ);
}

template<class V, class LA, class M>
double TKMeans<V, LA, M>::GetClusteringSSE() const {
	double Qual = 0.0;
	for (int DocId = 0; DocId < Assignment.Len(); DocId++) {
		double Sim = DCSim[Assignment[DocId]][DocId];
		Qual += (1.0 - Sim) * (1.0 - Sim);
	}
	return Qual;
}

template<class V, class LA, class M>
void TKMeans<V, LA, M>::PutUnitNorm(TFltV& Vec) {
	int WIds = Vec.Len();
	// get suqare-weight-sum
	double SqWgtSum = 0;
	for (int WIdN = 0; WIdN < WIds; WIdN++) {
		SqWgtSum += TMath::Sqr(Vec[WIdN]);
	}
	if (SqWgtSum > 0) {
		// normalize weights
		for (int WIdN = 0; WIdN < WIds; WIdN++) {
			Vec[WIdN] = (sdouble) sqrt(TMath::Sqr(Vec[WIdN]) / SqWgtSum);
		}
	}
}

template<class V, class LA, class M>
void TKMeans<V, LA, M>::PutUnitNorm(TIntFltKdV& Vec) {
	int WIds = Vec.Len();
	// get square-weight-sum
	double SqWgtSum = 0;
	for (int WIdN = 0; WIdN < WIds; WIdN++) {
		SqWgtSum += TMath::Sqr(Vec[WIdN].Dat);
	}
	if (SqWgtSum > 0) {
		// normalize weights
		for (int WIdN = 0; WIdN < WIds; WIdN++) {
			Vec[WIdN].Dat = (sdouble) sqrt(
					TMath::Sqr(Vec[WIdN].Dat) / SqWgtSum);
		}
	}
}

template<class V, class LA, class M>
TFltV& TKMeans<V, LA, M>::Centroid(int ClusterId) {
	return Centroids.ColV[ClusterId];
}

template<class V, class LA, class M>
void TKMeans<V, LA, M>::MakeCentroids(TVec<TIntV> & ClusterDocs,
		TIntV & CentroidSize) {
	// sum centroids
#pragma omp parallel for
	for (int CId = 0; CId < k; CId++) {
		const TIntV & DocIdV = ClusterDocs[CId];
		TFltV & Cen = Centroid(CId);
		for (int i = 0; i < DocIdV.Len(); i++) {
			const V& Vec = TDefaultMatrixAccess::GetDoc(DocVV, DocIdV[i]);
			LA::AddVec(1.0, Vec, Cen, Cen);
		}
		CentroidSize[CId] = ClusterDocs[CId].Len();
	}

}

template<class V, class LA, class M>
const TIntV& TKMeans<V, LA, M>::GetAssignment() const {
	return Assignment;
}

template<class V, class LA, class M>
void TKMeans<V, LA, M>::Apply() {
	bool stable = false;
	TIntV CentroidSize(k);
	//TFltVV DCSims(GetDocs(), k);

	for (int Iter = 0; (Iter < maxItr) && !stable; Iter++) {

		stable = true;
		// calculate
#pragma omp parallel for
		for (int CId = 0; CId < k; CId++) {
			DocVV->MultiplyT(GetCentroid(CId), DCSim[CId]);
		}
		//DocVV.MultiplyT()
		/*#pragma omp parallel for
		 for (int d = 0; d < GetDocs(); d++) {
		 const TIntFltKdV& Doc = GetDoc(d);
		 for (int c = 0; c < k; c++) {
		 const TFltV& Cluster = GetCentroid(c);
		 double Sim = TLinAlg::DotProduct(Cluster, Doc);
		 //printf("%2.4f\t", Sim);
		 DCSims(d,c) = Sim;
		 }
		 //printf("\n");
		 }*/

		// reassign
#pragma omp parallel for
		for (int DId = 0; DId < GetDocs(); DId++) {
			double BestSim = 0.0;
			int BestClust = 0;
			for (int c = 0; c < k; c++) {
				double Sim = DCSim[c][DId];
				Assert(Sim <= 1 + 1e-6);
				// just in case
				if (ShouldReassign(Sim, BestSim, DId)) {
					BestSim = Sim;
					BestClust = c;
				}
			}
			//printf("%2.4f\n", BestSim);

			int OldAssignment = Assignment[DId];
			Assignment[DId] = BestClust;
			if (BestClust != OldAssignment) {
				stable = false;
			}
		}

		if (stable)
			break;

		// reset
		for (int CId = 0; CId < k; CId++) {
			Centroid(CId).PutAll(0.0);
		}
		CentroidSize.PutAll(0);

		// gather docs
		TVec<TIntV> ClusterDocs(k);
		for (int DId = 0; DId < GetDocs(); DId++) {
			ClusterDocs[Assignment[DId]].Add(DId);
		}
		MakeCentroids(ClusterDocs, CentroidSize);

		// renormalize to actual centroids
		for (int CId = 0; CId < k; CId++) {
			TFltV & Cen = Centroid(CId);
			TLinAlg::MultiplyScalar(1.0 / CentroidSize[CId], Cen, Cen);
		}

		double Qual = GetClusteringCompactness();
		double QualSiz = GetClusteringQualityBySize(CentroidSize);
		double QualDc = GetClusteringQualityByDCSim();
		double Sse = GetClusteringSSE();
		TNotify::OnNotify(
				Notify,
				ntInfo,
				TStr::Fmt(
						"Iteration %d, qc:%2.4f, qs:%2.4f, qdc:%2.4f, sse:%2.4f ",
						Iter, Qual, QualSiz, QualDc, Sse));

		// renormalize to 2-norm to have fast calculation of DCSims
		for (int CId = 0; CId < k; CId++) {
			PutUnitNorm(Centroid(CId));
		}

	}
}
/**
 * KMeans++ magic
 * Chooses a number of Centers from the data set as follows:
 *  - One center is chosen randomly.
 *  - Now repeat k-1 times:
 *      - Repeat numLocalTries times:
 *          - Add a point x with probability proportional to the distance squared from x
 *            to the closest existing center
 *      - Add the point chosen above that results in the smallest potential.
 *
 *  @return indices of seed centroids
 **/
template<class V, class LA, class M>
void TKMeans<V, LA, M>::ChooseSmartCenters(int numLocalTries, TIntV& Centers) {

	IAssert(GetDocs() > 0);
	IAssert(GetDocs() > k);

	int center = 0;

	Centers.Gen(k);
	TFltV SqDistances(GetDocs());

	// Choose one random center and initialize closest sq. distances
	int Index = (int) (Rnd.GetUniDev() * GetDocs());
	Centers[center++] = Index;

	// Get initial potential
	double potential = 0.0;
	const V& FirstCenter = TDefaultMatrixAccess::GetDoc(DocVV, Index);

	for (int i = 0; i < GetDocs(); i++) {
		const V& Doc = TDefaultMatrixAccess::GetDoc(DocVV, i);
		double Dist = 1.0 - LA::DotProduct(Doc, FirstCenter);
		double SqDist = Dist * Dist;

		SqDistances[i] = SqDist;
		potential += SqDist;
	}

	// Choose each center
	for (int i = 1; i < k; i++) {

		// Repeat several trials
		double bestNewCandidate = -1.0;
		int bestNewIndex = 0;
		for (int j = 0; j < numLocalTries; j++) {

			// Choose our center - have to be slightly careful to return a valid answer even accounting
			// for possible rounding errors
			Index = SelectCentroidCenter(potential, SqDistances);

			// Compute the new potential
			double newCandidate = GetCandidatePotential(SqDistances, Index);

			// Store the best result
			if (bestNewCandidate < 0 || newCandidate < bestNewCandidate) {
				bestNewCandidate = newCandidate;
				bestNewIndex = Index;
			}
		}

		// Add the appropriate center
		Centers[center++] = bestNewIndex;
		potential = bestNewCandidate;

		const V& BestCand = TDefaultMatrixAccess::GetDoc(DocVV, bestNewIndex);

		for (int j = 0; j < GetDocs(); j++) {
			const V& Doc = TDefaultMatrixAccess::GetDoc(DocVV, j);
			double Dist = 1.0 - LA::DotProduct(Doc, BestCand);
			const TFlt SqDist = Dist * Dist;

			SqDistances[j] = TMath::Mn(SqDist, SqDistances[j]);
		}
	}
}

/**
 * Get sum of nearest distances to other seeds.
 *
 * @param SqDistances, D(x_i)^2 for each x_i
 * @param Index, Index of candidate data point
 * @return data point potential
 */
template<class V, class LA, class M>
double TKMeans<V, LA, M>::GetCandidatePotential(const TFltV& SqDistances,
		int Index) const {
	double newCandidate = 0;
	const V& BestCand = TDefaultMatrixAccess::GetDoc(DocVV, Index);

	for (int i = 0; i < GetDocs(); i++) {
		const V& Doc = TDefaultMatrixAccess::GetDoc(DocVV, i);
		double Dist = 1.0 - LA::DotProduct(Doc, BestCand);
		const TFlt SqDist = Dist * Dist;

		newCandidate += TMath::Mn(SqDist, SqDistances[i]);
	}
	return newCandidate;
}

/**
 * Returns data point x' with probability D(x')^2 / sum{D(x)^2)
 * @param potential, equal to sum{D(x)^2)
 * @param SqDistances, D(x_i)^2 for each x_i
 * @param rnd, randomizer
 * @return Index of chosen data point
 */
template<class V, class LA, class M>
int TKMeans<V, LA, M>::SelectCentroidCenter(double potential,
		const TFltV& SqDistances) {
	int i;
	// Get random value
	double r = Rnd.GetUniDev() * potential;
	// Subtract distances from this value and see where we end up
	for (i = 0; i < GetDocs() - 1; i++) {
		if (r <= SqDistances[i]) {
			break;
		}
		r -= SqDistances[i];
	}
	// i is the lucky winner
	return i;
}


#endif
