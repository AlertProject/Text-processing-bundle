#ifndef tmine_h
#define tmine_h

#include "../base.h"
#include "dmine.h"

#include "tmine/stopword.h"
#include "tmine/stemming.h"
#include "tmine/phrase.h"
#include "tmine/tokenizer.h"

#include "tmine/cpdoc.h"

// Bag-Of-Words
#include "tmine/bowbs.h"
#include "tmine/bowfl.h"
#include "tmine/bowlearn.h"
#include "tmine/bowmd.h"
#include "tmine/bowclust.h"

// feature-generator
#include "tmine/ftrgen.h"

// Linear-Algebra
#include "tmine/bowlinalg.h"

// Bilingual-base
#include "tmine/biling.h"

// SVM
#include "tmine/svmPrLoqo.h"
#include "tmine/svmbasic.h"
#include "tmine/strkernel.h"
#include "tmine/svmmodels.h"

// Kernel-Methods
#include "tmine/kernelmethods.h"
#include "tmine/semspace.h"
#include "tmine/ccar.h"

// kmeans++
#include "tmine/kmpp.h"

// Logistic-Regresion
#include "tmine/logreg.h"

// Active-Learning
#include "tmine/bowactlearn.h"

// Light-Weight-Ontologies
#include "tmine/ontolight.h"

// visualization
#include "tmine/vizmap.h"

// special datasets
#include "tmine/ciawfb.h"
#include "tmine/dmoz.h"
#include "tmine/acquis.h"

#endif
