#ifndef _TemplateHelpers_H_
#define _TemplateHelpers_H_

#include <boost/shared_ptr.hpp>

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/DataArrays/IDataArray.h"

/*
* @brief This file contains a namespace and some C++ Functors that help to reduce the amout of code that filters
* need when the filter needs to determine if an IDataArray is of a certain primitive type.
*/

namespace TemplateHelpers
{

  /**
* @brief Define from Error Codes
*/
  namespace Errors
  {
    const int UnsupportedType(-401);
    const int MissingDataContainer(-402);
    const int MissingAttributeMatrix(-403);
    const int MissingArray(-404);
  }

  /**
* @brief This class (functor) simply returns true or false if the IDataArray can be downcast to a certain DataArray type
* parameterized by the template parameter T.
*/
  template<typename T>
  class CanDynamicCast
  {
    public:
      CanDynamicCast() {}
      virtual ~CanDynamicCast() {}
      bool operator()(IDataArray::Pointer p)
      {
        return (boost::dynamic_pointer_cast<T>(p).get() != NULL);
      }
  };


  /**
   * @brief The CreateNonPrereqArrayFromArrayType class will create a DataArray of the same type as another DataArray and attach it to
   * a supplied data container.
   */
  class CreateNonPrereqArrayFromArrayType
  {
    public:
      CreateNonPrereqArrayFromArrayType(){}
      ~CreateNonPrereqArrayFromArrayType(){}

      IDataArray::WeakPointer operator()(AbstractFilter* f, DataArrayPath arrayPath, QVector<size_t> compDims, IDataArray::Pointer sourceArrayType)
      {

        IDataArray::Pointer ptr = IDataArray::NullPointer();

        if(CanDynamicCast<FloatArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>, AbstractFilter, float>(f, arrayPath, 0, compDims);
        }
        else if(CanDynamicCast<DoubleArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<double>, AbstractFilter, double>(f, arrayPath, 0, compDims);
        }
        else if(CanDynamicCast<Int8ArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int8_t>, AbstractFilter, int8_t>(f, arrayPath, 0, compDims);
        }
        else if(CanDynamicCast<UInt8ArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint8_t>, AbstractFilter, uint8_t>(f, arrayPath, 0, compDims);
        }
        else if(CanDynamicCast<Int16ArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int16_t>, AbstractFilter, int16_t>(f, arrayPath, 0, compDims);
        }
        else if(CanDynamicCast<UInt16ArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint16_t>, AbstractFilter, uint16_t>(f, arrayPath, 0, compDims);
        }
        else if(CanDynamicCast<Int32ArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter, int32_t>(f, arrayPath, 0, compDims);
        }
        else if(CanDynamicCast<UInt32ArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint32_t>, AbstractFilter, uint32_t>(f, arrayPath, 0, compDims);
        }
        else if(CanDynamicCast<Int64ArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int64_t>, AbstractFilter, int64_t>(f, arrayPath, 0, compDims);
        }
        else if(CanDynamicCast<UInt64ArrayType>()(sourceArrayType) ) {
          ptr = f->getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint64_t>, AbstractFilter, uint64_t>(f, arrayPath, 0, compDims);
        }
        else {
          QString msg = QObject::tr("The created array '%1' is of unsupported type. The following types are supported: %3").arg(arrayPath.getDataArrayName()).arg(DREAM3D::TypeNames::SupportedTypeList);
          f->setErrorCondition(Errors::UnsupportedType);
          f->notifyErrorMessage(f->getHumanLabel(), msg, f->getErrorCondition());
        }
        return ptr;
      }

  };

  /**
   * @brief The CreateArrayFromArrayType class will create a DataArray of the same type as another DataArray but not attach
   * it to any data container.
   */
   class CreateArrayFromArrayType
   {
     public:
       CreateArrayFromArrayType(){}
       ~CreateArrayFromArrayType(){}

       IDataArray::Pointer operator()(AbstractFilter* f, size_t numTuples, QVector<size_t> compDims, QString arrayName, bool allocate, IDataArray::Pointer sourceArrayType)
       {
         CreateArrayFromArrayType classInstance;
         QVector<size_t> tupleDims(1, numTuples);
         return classInstance(f, tupleDims, compDims, arrayName, allocate, sourceArrayType);
       }

       IDataArray::Pointer operator()(AbstractFilter* f, QVector<size_t> tupleDims, QVector<size_t> compDims, QString arrayName, bool allocate, IDataArray::Pointer sourceArrayType)
       {
         IDataArray::Pointer ptr = IDataArray::NullPointer();

         if(CanDynamicCast<FloatArrayType>()(sourceArrayType) ) {
           ptr = FloatArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(CanDynamicCast<DoubleArrayType>()(sourceArrayType) ) {
           ptr = DoubleArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(CanDynamicCast<Int8ArrayType>()(sourceArrayType) ) {
           ptr = Int8ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(CanDynamicCast<UInt8ArrayType>()(sourceArrayType) ) {
           ptr = UInt8ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(CanDynamicCast<Int16ArrayType>()(sourceArrayType) ) {
           ptr = Int16ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(CanDynamicCast<UInt16ArrayType>()(sourceArrayType) ) {
           ptr = UInt16ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(CanDynamicCast<Int32ArrayType>()(sourceArrayType) ) {
           ptr = Int32ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(CanDynamicCast<UInt32ArrayType>()(sourceArrayType) ) {
           ptr = UInt32ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(CanDynamicCast<Int64ArrayType>()(sourceArrayType) ) {
           ptr = Int64ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(CanDynamicCast<UInt64ArrayType>()(sourceArrayType) ) {
           ptr = UInt64ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else {
          QString msg = QObject::tr("The created array is of unsupported type.");
          f->setErrorCondition(Errors::UnsupportedType);
          f->notifyErrorMessage(f->getHumanLabel(), msg, f->getErrorCondition());
         }
         return ptr;
       }
   };

   /**
    * @brief The CreateArrayFromType class will create a DataArray matching the type supplied as a QString.  Available types are listed
    * in Constants.h.
    */
   class CreateArrayFromType
   {
     public:
       CreateArrayFromType(){}
       ~CreateArrayFromType(){}

       IDataArray::Pointer operator()(AbstractFilter* f, size_t numTuples, QVector<size_t> compDims, QString arrayName, bool allocate, QString type)
       {
         CreateArrayFromType classInstance;
         QVector<size_t> tupleDims(1, numTuples);
         return classInstance(f, tupleDims, compDims, arrayName, allocate, type);
       }

       IDataArray::Pointer operator()(AbstractFilter* f, QVector<size_t> tupleDims, QVector<size_t> compDims, QString arrayName, bool allocate, QString type)
       {
         IDataArray::Pointer ptr = IDataArray::NullPointer();

         if(type.compare(DREAM3D::TypeNames::Float) == 0) {
           ptr = FloatArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(type.compare(DREAM3D::TypeNames::Double) == 0) {
           ptr = DoubleArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(type.compare(DREAM3D::TypeNames::Int8) == 0) {
           ptr = Int8ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(type.compare(DREAM3D::TypeNames::UInt8) == 0) {
           ptr = UInt8ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(type.compare(DREAM3D::TypeNames::Int16) == 0) {
           ptr = Int16ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(type.compare(DREAM3D::TypeNames::UInt16) == 0) {
           ptr = UInt16ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(type.compare(DREAM3D::TypeNames::Int32) == 0) {
           ptr = Int32ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(type.compare(DREAM3D::TypeNames::UInt32) == 0) {
           ptr = UInt32ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(type.compare(DREAM3D::TypeNames::Int64) == 0) {
           ptr = Int64ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else if(type.compare(DREAM3D::TypeNames::UInt64) == 0) {
           ptr = UInt64ArrayType::CreateArray(tupleDims, compDims, arrayName, allocate);
         }
         else {
           QString msg = QObject::tr("The created array is of unsupported type.");
           f->setErrorCondition(Errors::UnsupportedType);
           f->notifyErrorMessage(f->getHumanLabel(), msg, f->getErrorCondition());
         }
         return ptr;
       }
   };

   /**
   * @brief The GetPrereqArrayFromPath class will return a pointer to a DataArray of unknown type given the path.
   */
  template<typename FilterClass, typename DataContainerClass>
  class GetPrereqArrayFromPath
  {
    public:
      GetPrereqArrayFromPath(){}
      virtual ~GetPrereqArrayFromPath(){}

      IDataArray::WeakPointer operator()(FilterClass* f, DataArrayPath arrayPath, QVector<size_t>& compDims)
      {
        IDataArray::Pointer retPtr = IDataArray::NullPointer();
        DataContainerClass* volDataCntr = f->getDataContainerArray()->template getPrereqDataContainer<DataContainerClass, FilterClass>(f, arrayPath.getDataContainerName(), false);
        if(f->getErrorCondition() < 0 || NULL == volDataCntr) {
          QString ss = QObject::tr("The Data Container '%1' does not exist").arg(arrayPath.getDataContainerName());
          f->setErrorCondition(Errors::MissingDataContainer);
          f->notifyErrorMessage(f->getHumanLabel(), ss, f->getErrorCondition());
          return retPtr;
        }
        AttributeMatrix::Pointer cell_attr_matrix = volDataCntr->template getPrereqAttributeMatrix<FilterClass>(f, arrayPath.getAttributeMatrixName(), Errors::MissingAttributeMatrix);
        if(f->getErrorCondition() < 0 || NULL == cell_attr_matrix.get()) {
          QString ss = QObject::tr("The Attribute Matrix '%1' does not exist").arg(arrayPath.getAttributeMatrixName());
          f->setErrorCondition(Errors::MissingAttributeMatrix);
          f->notifyErrorMessage(f->getHumanLabel(), ss, f->getErrorCondition());
          return retPtr;
        }
        IDataArray::Pointer templ_ptr = cell_attr_matrix->getAttributeArray(arrayPath.getDataArrayName());
        if(NULL == templ_ptr.get()) {
          QString ss = QObject::tr("The input array '%1' was not found in the AttributeMatrix '%2'.").arg(arrayPath.getDataArrayName()).arg(arrayPath.getAttributeMatrixName());
          f->setErrorCondition(Errors::MissingArray);
          f->notifyErrorMessage(f->getHumanLabel(), ss, f->getErrorCondition());
          return retPtr;
        }
        if(compDims.isEmpty()) {
          compDims = templ_ptr->getComponentDimensions();
        }

        IDataArray::Pointer i_data_array = cell_attr_matrix->getAttributeArray(arrayPath.getDataArrayName());
        if(CanDynamicCast<Int8ArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<int8_t>, AbstractFilter>(f, arrayPath, compDims);
        }
        else if(CanDynamicCast<UInt8ArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<uint8_t>, AbstractFilter>(f, arrayPath, compDims);
        }
        else if(CanDynamicCast<Int16ArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<int16_t>, AbstractFilter>(f, arrayPath, compDims);
        }
        else if(CanDynamicCast<UInt16ArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<uint16_t>, AbstractFilter>(f, arrayPath, compDims);
        }
        else if(CanDynamicCast<Int32ArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(f, arrayPath, compDims);
        }
        else if(CanDynamicCast<UInt32ArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<uint32_t>, AbstractFilter>(f, arrayPath, compDims);
        }
        else if(CanDynamicCast<Int64ArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<int64_t>, AbstractFilter>(f, arrayPath, compDims);
        }
        else if(CanDynamicCast<UInt64ArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<uint64_t>, AbstractFilter>(f, arrayPath, compDims);
        }
        else if(CanDynamicCast<FloatArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<float>, AbstractFilter>(f, arrayPath, compDims);
        }
        else if(CanDynamicCast<DoubleArrayType>()(i_data_array) ) {
          retPtr = f->getDataContainerArray()->template getPrereqArrayFromPath<DataArray<double>, AbstractFilter>(f, arrayPath, compDims);
        }
        else
        {
          QString ss = QObject::tr("The input array %1 is of unsupported type '%2'. The following types are supported: %3").arg(arrayPath.getDataArrayName()).arg(  retPtr->getTypeAsString()).arg(DREAM3D::TypeNames::SupportedTypeList);
          f->setErrorCondition(Errors::UnsupportedType);
          f->notifyErrorMessage(f->getHumanLabel(), ss, f->getErrorCondition());
        }
        return retPtr;

      }


    private:
      GetPrereqArrayFromPath(const GetPrereqArrayFromPath&); // Copy Constructor Not Implemented
      void operator=(const GetPrereqArrayFromPath&); // Operator '=' Not Implemented
  };

}

#endif /* _TemplateHelpers_H_ */
