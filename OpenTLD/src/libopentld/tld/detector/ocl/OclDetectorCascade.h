//Cao Yilun
//2014.10.29
#ifndef OCLDETECTORCASCADE_H_
#define OCLDETECTORCASCADE_H_

#include "IDetectorCascade.h"
#include "DetectionResult.h"
//#include "ForegroundDetector.h"
#include "VarianceFilter.h"
#include "EnsembleClassifier.h"
#include "Clustering.h"
#include "NNClassifier.h"



namespace tld
{

//namespace ocl
//{

class OclDetectorCascade : public IDetectorCascade
{
public:

    OclDetectorCascade();
    virtual ~OclDetectorCascade();

    virtual void init();
    virtual void initWindowOffsets();
    virtual void initWindowsAndScales();
    virtual void propagateMembers();
    virtual void release();
    virtual void cleanPreviousData();
    virtual void detect(const cv::Mat &img);
};

//} /* namespace ocl */

} /* namespace tld */
#endif /* OCLDETECTORCASCADE_H_ */
