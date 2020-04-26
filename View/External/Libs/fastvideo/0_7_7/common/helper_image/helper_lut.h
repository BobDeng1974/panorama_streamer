/*
Copyright 2012-2014 Fastvideo, Ltd. All rights reserved.

Please refer to the Fastvideo Standard License Agreement (SLA), associated
 with this source code for terms and conditions that govern your use of
 this software. Any use, reproduction, disclosure, or distribution of
 this software and related documentation outside the terms of the SLA is strictly prohibited.

IN NO EVENT SHALL FASTVIDEO BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF FASTVIDEO HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FASTVIDEO SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". FASTVIDEO HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/
#ifndef HELPER_LUT_H
#define HELPER_LUT_H

#include <fstream>
#include <iostream>

#include "fastvideo_sdk.h"
#include "memory_interface.h"
#include "helper_common.h"

template<class Allocator>
inline fastStatus_t
__loadLut(const char *file, std::unique_ptr<unsigned char, Allocator> &data, unsigned size) {
	FILE *fp = NULL;

    if (FOPEN_FAIL(FOPEN(fp, file, "r")))
        return FAST_IO_ERROR;

	CHECK_FAST_ALLOCATION( data.reset( (unsigned char *)Allocator::template allocate<unsigned char>(size) ) );
	//CHECK_FAST_ALLOCATION( fastMalloc((void **)data, size) );

	unsigned i = 0;
	while ( i < size && !feof(fp) ) {
		fscanf(fp, "%d", &data.get()[i]);
		i++;
	}

	if ( i != size ) {
		return FAST_IO_ERROR;
	}

	fclose(fp);
	return FAST_OK;
}

#endif
