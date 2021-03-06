/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __itkGradientDescentOptimizerBasev4_h
#define __itkGradientDescentOptimizerBasev4_h

#include "itkObjectToObjectOptimizerBase.h"
#include "itkGradientDescentOptimizerBasev4ModifyGradientByScalesThreader.h"
#include "itkGradientDescentOptimizerBasev4ModifyGradientByLearningRateThreader.h"

namespace itk
{
/** \class GradientDescentOptimizerBasev4
 *  \brief Abstract base class for gradient descent-style optimizers.
 *
 * Gradient modification is threaded in \c ModifyGradient.
 *
 * Derived classes must override \c ModifyGradientByScalesOverSubRange,
 * \c ModifyGradientByLearningRateOverSubRange and \c ResumeOptimization.
 *
 * \ingroup ITKOptimizersv4
 */

class ITK_EXPORT GradientDescentOptimizerBasev4
  : public ObjectToObjectOptimizerBase
{
public:
  /** Standard class typedefs. */
  typedef GradientDescentOptimizerBasev4 Self;
  typedef ObjectToObjectOptimizerBase    Superclass;
  typedef SmartPointer< Self >           Pointer;
  typedef SmartPointer< const Self >     ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(GradientDescentOptimizerBasev4, ObjectToObjectOptimizerBase);

  /** Codes of stopping conditions. */
  typedef enum {
    MAXIMUM_NUMBER_OF_ITERATIONS,
    COSTFUNCTION_ERROR,
    UPDATE_PARAMETERS_ERROR,
    STEP_TOO_SMALL,
    QUASI_NEWTON_STEP_ERROR,
    CONVERGENCE_CHECKER_PASSED,
    OTHER_ERROR
    } StopConditionType;

  /** Stop condition return string type */
  typedef std::string                            StopConditionReturnStringType;

  /** Stop condition internal string type */
  typedef std::ostringstream                     StopConditionDescriptionType;

  /** Metric type over which this class is templated */
  typedef Superclass::MetricType                    MetricType;
  typedef MetricType::Pointer                       MetricTypePointer;

  /** Derivative type */
  typedef MetricType::DerivativeType                DerivativeType;

  /** Measure type */
  typedef Superclass::MeasureType                   MeasureType;

  /** Internal computation type, for maintaining a desired precision */
  typedef Superclass::InternalComputationValueType InternalComputationValueType;

  /** Get the most recent gradient values. */
  itkGetConstReferenceMacro( Gradient, DerivativeType );

  /** Get stop condition enum */
  itkGetConstReferenceMacro(StopCondition, StopConditionType);

  /** Set the number of iterations. */
  itkSetMacro(NumberOfIterations, SizeValueType);

  /** Get the number of iterations. */
  itkGetConstReferenceMacro(NumberOfIterations, SizeValueType);

  /** Get the current iteration number. */
  itkGetConstMacro(CurrentIteration, SizeValueType);

  /** Resume optimization.
   * This runs the optimization loop, and allows continuation
   * of stopped optimization */
  virtual void ResumeOptimization() = 0;

  /** Stop optimization. The object is left in a state so the
   * optimization can be resumed by calling ResumeOptimization. */
  virtual void StopOptimization(void);

  /** Get the reason for termination */
  virtual const StopConditionReturnStringType GetStopConditionDescription() const;

  /** Modify the gradient in place, to advance the optimization.
   * This call performs a threaded modification for transforms with
   * local support (assumed to be dense). Otherwise the modification
   * is performed w/out threading.
   * See EstimateLearningRate() to perform optionaly learning rate
   * estimation.
   * At completion, m_Gradient can be used to update the transform
   * parameters. Derived classes may hold additional results in
   * other member variables.
   *
   * \sa EstimateLearningRate()
   */
  virtual void ModifyGradientByScales();
  virtual void ModifyGradientByLearningRate();

protected:

  /** Default constructor */
  GradientDescentOptimizerBasev4();
  virtual ~GradientDescentOptimizerBasev4();

  friend class GradientDescentOptimizerBasev4ModifyGradientByScalesThreader;
  friend class GradientDescentOptimizerBasev4ModifyGradientByLearningRateThreader;

  typedef GradientDescentOptimizerBasev4ModifyGradientByScalesThreader::IndexRangeType IndexRangeType;

  GradientDescentOptimizerBasev4ModifyGradientByScalesThreader::Pointer       m_ModifyGradientByScalesThreader;
  GradientDescentOptimizerBasev4ModifyGradientByLearningRateThreader::Pointer m_ModifyGradientByLearningRateThreader;

  /** Derived classes define this worker method to modify the gradient by scales.
   * Modifications must be performed over the index range defined in
   * \c subrange.
   * Called from ModifyGradientByScales(), either directly or via threaded
   * operation. */
  virtual void ModifyGradientByScalesOverSubRange( const IndexRangeType& subrange ) = 0;

  /** Derived classes define this worker method to modify the gradient by learning rates.
   * Modifications must be performed over the index range defined in
   * \c subrange.
   * Called from ModifyGradientByLearningRate(), either directly or via threaded
   * operation. */
  virtual void ModifyGradientByLearningRateOverSubRange( const IndexRangeType& subrange ) = 0;

  /* Common variables for optimization control and reporting */
  bool                          m_Stop;
  StopConditionType             m_StopCondition;
  StopConditionDescriptionType  m_StopConditionDescription;
  SizeValueType                 m_NumberOfIterations;
  SizeValueType                 m_CurrentIteration;

  /** Current gradient */
  DerivativeType     m_Gradient;
  virtual void PrintSelf(std::ostream & os, Indent indent) const;

private:
  GradientDescentOptimizerBasev4( const Self & ); //purposely not implemented
  void operator=( const Self& ); //purposely not implemented

};

} // end namespace itk

#endif
