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
#ifndef __itkBSplineSyNImageRegistrationMethod_h
#define __itkBSplineSyNImageRegistrationMethod_h

#include "itkSyNImageRegistrationMethod.h"

#include "itkBSplineSmoothingOnUpdateDisplacementFieldTransform.h"

namespace itk
{
//Forward-declare these because of module dependency conflict.
//They will soon be moved to a different module, at which
// time this can be removed.
template <unsigned int VDimension, class TDataHolder>
class ImageToData;
template <class TDataHolder>
class Array1DToData;

/** \class BSplineSyNImageRegistrationMethod
 * \brief Interface method for the performing greedy B-spline SyN image registration.
 *
 * For greedy SyN we use \c m_Transform to map the time-parameterized middle
 * image to the fixed image (and vice versa using
 * \c m_Transform->GetInverseDisplacementField() ).  We employ another ivar,
 * \c m_InverseTransform, to map the time-parameterized middle image to the
 * moving image.
 *
 * Output: The output is the updated transform which has been added to the
 * composite transform.
 *
 * \author Nick Tustison
 * \author Brian Avants
 *
 * \ingroup ITKRegistrationMethodsv4
 */
template<typename TFixedImage, typename TMovingImage, typename TOutputTransform =
  BSplineSmoothingOnUpdateDisplacementFieldTransform<double, GetImageDimension<TFixedImage>::ImageDimension> >
class ITK_EXPORT BSplineSyNImageRegistrationMethod
: public SyNImageRegistrationMethod<TFixedImage, TMovingImage, TOutputTransform>
{
public:
  /** Standard class typedefs. */
  typedef BSplineSyNImageRegistrationMethod                                           Self;
  typedef SyNImageRegistrationMethod<TFixedImage, TMovingImage, TOutputTransform>     Superclass;
  typedef SmartPointer<Self>                                                          Pointer;
  typedef SmartPointer<const Self>                                                    ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** ImageDimension constants */
  itkStaticConstMacro( ImageDimension, unsigned int, TFixedImage::ImageDimension );

  /** Run-time type information (and related methods). */
  itkTypeMacro( BSplineSyNImageRegistrationMethod, SyNImageRegistrationMethod );

  /** Input typedefs for the images and transforms. */
  typedef TFixedImage                                                 FixedImageType;
  typedef typename FixedImageType::Pointer                            FixedImagePointer;
  typedef TMovingImage                                                MovingImageType;
  typedef typename MovingImageType::Pointer                           MovingImagePointer;

  /** Metric and transform typedefs */
  typedef typename Superclass::MetricType                             MetricType;
  typedef typename MetricType::Pointer                                MetricPointer;
  typedef typename MetricType::VirtualImageType                       VirtualImageType;
  typedef typename MetricType::MeasureType                            MeasureType;

  typedef typename Superclass::NumberOfIterationsArrayType            NumberOfIterationsArrayType;

  typedef TOutputTransform                                            OutputTransformType;
  typedef typename OutputTransformType::Pointer                       OutputTransformPointer;
  typedef typename OutputTransformType::ScalarType                    RealType;
  typedef typename OutputTransformType::DerivativeType                DerivativeType;
  typedef typename DerivativeType::ValueType                          DerivativeValueType;
  typedef typename OutputTransformType::DisplacementFieldType         DisplacementFieldType;
  typedef typename DisplacementFieldType::Pointer                     DisplacementFieldPointer;
  typedef typename DisplacementFieldType::PixelType                   DisplacementVectorType;

  typedef DisplacementFieldToBSplineImageFilter
    <DisplacementFieldType, DisplacementFieldType>                    BSplineFilterType;
  typedef typename BSplineFilterType::ArrayType                       ArrayType;

  typedef typename Superclass::CompositeTransformType                 CompositeTransformType;
  typedef typename CompositeTransformType::TransformType              TransformBaseType;

  typedef typename Superclass::DecoratedOutputTransformType           DecoratedOutputTransformType;
  typedef typename DecoratedOutputTransformType::Pointer              DecoratedOutputTransformPointer;

protected:
  BSplineSyNImageRegistrationMethod();
  virtual ~BSplineSyNImageRegistrationMethod();

  /** Handle optimization internally */
  virtual void StartOptimization();

  virtual void InitializeRegistrationAtEachLevel( const SizeValueType );

  virtual DisplacementFieldPointer ComputeUpdateField( const TFixedImage *, const TransformBaseType *, const TMovingImage *, const TransformBaseType *, MeasureType & );
  virtual DisplacementFieldPointer BSplineSmoothDisplacementField( const DisplacementFieldType *, const ArrayType & );

private:
  BSplineSyNImageRegistrationMethod( const Self & );   //purposely not implemented
  void operator=( const Self & );               //purposely not implemented
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBSplineSyNImageRegistrationMethod.hxx"
#endif

#endif
