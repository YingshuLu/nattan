/** Copyright (C) Nattan, Inc - All Rights Reserved
 ** Unauthorized copying of this file, via any medium is strictly prohibited
 ** Proprietary and confidential
 ** Written by Kam Lu <yingshulu@gmail.com>, July 2019
 **/

#ifndef NATTAN_CORE_BUFFER_H
#define NATTAN_CORE_BUFFER_H

#include <algorithm>
#include "core/MemoryData.h"
#include "base/Writeable.h"
#include "log/Log.h"

namespace nattan {

static const size_t kBufferPrefixed = 8;
class Buffer: public MemoryData {

public:

	Buffer(const unsigned int init_capacity = 1024 * 12): fReadIdx(kBufferPrefixed), fWriteIdx(kBufferPrefixed) {
		fData.reserve(init_capacity > kBufferPrefixed? init_capacity : 1024);
		fData.insert(fData.end(), kBufferPrefixed, 'F');
	}

	size_t capacity() { return fData.capacity() - kBufferPrefixed; }
	size_t length() { return fWriteIdx - fReadIdx; }

	ssize_t read(char* buf, size_t buf_len) {
		size_t len = std::min(buf_len, length());
		//empty
		if (len == 0) return 0;
		memcpy(buf, data(), len);
		fReadIdx += len;
		tryArrange();
		return len;
	}

	ssize_t write(const char* buf, size_t buf_len) {
		if (right() < buf_len) {
			if (right() + left() >= buf_len) arrange();
			else {
				size_t rz = capacity();
				while(rz - length() <= buf_len) {
					rz <<= 1;
				}
				resize(rz);
			}
		}
		fData.insert(fData.begin() + fWriteIdx, buf, buf + buf_len);
		fWriteIdx += buf_len;
		return buf_len;
	}

	using Data::write;

	ssize_t sendTo(Writeable& w) {
		if (length() == 0) return 0;
		ssize_t res = w.write(data(), length());		
		if (res > 0) shrink(res);
		return res;
	}

	ssize_t fill(size_t len) {
		return fill(nullptr, len);
	}

	ssize_t fill(char c, size_t len) {
		return fill(&c, len);
	}

	char* data() {
		return fData.data() + fReadIdx;
	}

	char* end() {
		return fData.data() + fWriteIdx;
	}

	void clear() {
		fReadIdx = fWriteIdx = kBufferPrefixed;
	}

	size_t shrink(const size_t len) {
		int l = std::min(length(), len);
		fReadIdx += l;
		tryArrange();
		return l;
	}

	bool tryArrange() {
		if (fReadIdx == fWriteIdx) {
			fReadIdx = kBufferPrefixed;
			fWriteIdx = kBufferPrefixed;
			return true;
		}	

		if (((double) fReadIdx * 1.0 / (double)capacity()) > 0.88) {
			arrange();
			return true;
		}
		return false;
	}

	void expand() {
		resize(fData.capacity() * 2);
	}

	size_t right() { return  capacity() - fWriteIdx; }
	size_t left() { return fReadIdx - kBufferPrefixed; }


private:
	ssize_t fill(char* c, size_t len) {
		if (right() < len) {
			resize(length() + len);
		}

		if (NULL != c) {
			fData.insert(fData.begin() + fWriteIdx, len, *c);
		}
		fWriteIdx += len;
		return len;	
	}
	
	void resize(size_t size) {
		fData.resize(size);
	}

	void arrange() {
		DBG_LOG("buffer arrange");
		size_t i = kBufferPrefixed, k = fReadIdx;
		while(k < fWriteIdx) {
			fData[i++] = fData[k++];
		}

		fReadIdx = kBufferPrefixed;
		fWriteIdx = i;
	}

private:
	std::vector<char> fData;
	std::size_t fReadIdx;
	std::size_t fWriteIdx;
};

} //namespace nattan

#endif
