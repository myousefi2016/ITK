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

#ifndef __itkLevelSetImage_hxx
#define __itkLevelSetImage_hxx

#include "itkLevelSetImage.h"

namespace itk
{
// ----------------------------------------------------------------------------
template< class TInput, unsigned int VDimension, typename TOutput >
LevelSetImage< TInput, VDimension, TOutput >
::LevelSetImage()
{
  this->m_NeighborhoodScales.Fill( NumericTraits< OutputRealType >::One );
}

// ----------------------------------------------------------------------------
template< class TInput, unsigned int VDimension, typename TOutput >
LevelSetImage< TInput, VDimension, TOutput >
::~LevelSetImage()
{
}

}
#endif // __itkLevelSetImage_hxx
