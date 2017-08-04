#ifndef __DAO_BINARY_FIELD_UTIL_H__
#define __DAO_BINARY_FIELD_UTIL_H__
#pragma once

namespace dao_binary_field_util
{
    template <class TOleVariant>
	inline bool PutBinaryIntoVariant(TOleVariant *ovData, byte *pBuf, size_t cBufLen)
	{
		 bool fRetVal = false;

		 VARIANT var;
		 VariantInit(&var);  //Initialize our variant

		 //Set the type to an array of unsigned chars (OLE SAFEARRAY)
		 var.vt = VT_ARRAY | VT_UI1;

		 //Set up the bounds structure
		 SAFEARRAYBOUND  rgsabound[1];

		 rgsabound[0].cElements = cBufLen;
		 rgsabound[0].lLbound = 0;

		 //Create an OLE SAFEARRAY
		 var.parray = SafeArrayCreate(VT_UI1,1,rgsabound);
	 
		 if (var.parray != NULL)
		 {
			void *pArrayData = NULL;
		
			//Get a safe pointer to the array
			SafeArrayAccessData(var.parray, &pArrayData);

			//Copy bitmap to it
			memcpy(pArrayData, pBuf, cBufLen);

			//Unlock the variant data
			SafeArrayUnaccessData(var.parray);

			*ovData = var;  // Create a COleVariant based on our variant
			VariantClear(&var);
			fRetVal = true;
		 }

		 return fRetVal;
	}

	template <class TOleVariant>
	inline bool GetBinaryFromVariant(TOleVariant &ovData, byte **ppBuf, size_t *pcBufLen)
	{
		 bool fRetVal = false;

	   //Binary data is stored in the variant as an array of unsigned char
		 if (ovData.vt == (VT_ARRAY|VT_UI1))  // (OLE SAFEARRAY)
		 {
		   //Retrieve size of array
		   *pcBufLen = ovData.parray->rgsabound[0].cElements;

		   *ppBuf = new BYTE[*pcBufLen]; //Allocate a buffer to store the data
		   if (*ppBuf != NULL)
		   {
			 void * pArrayData;

			 //Obtain safe pointer to the array
			 SafeArrayAccessData(ovData.parray,&pArrayData);

			 //Copy the bitmap into our buffer
			 memcpy(*ppBuf, pArrayData, *pcBufLen);
			
			 //Unlock the variant data
			 SafeArrayUnaccessData(ovData.parray);
			 fRetVal = true;
		   }
		 }

		 return fRetVal;
	}
};

#endif