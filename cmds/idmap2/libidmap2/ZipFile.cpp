/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>
#include <string>

#include "idmap2/Result.h"
#include "idmap2/ZipFile.h"

namespace android {
namespace idmap2 {

std::unique_ptr<MemoryChunk> MemoryChunk::Allocate(size_t size) {
  void* ptr = ::operator new(sizeof(MemoryChunk) + size);
  std::unique_ptr<MemoryChunk> chunk(reinterpret_cast<MemoryChunk*>(ptr));
  chunk->size = size;
  return chunk;
}

std::unique_ptr<const ZipFile> ZipFile::Open(const std::string& path) {
  ::ZipArchiveHandle handle;
  int32_t status = ::OpenArchive(path.c_str(), &handle);
  if (status != 0) {
    return nullptr;
  }
  return std::unique_ptr<ZipFile>(new ZipFile(handle));
}

ZipFile::~ZipFile() {
  ::CloseArchive(handle_);
}

std::unique_ptr<const MemoryChunk> ZipFile::Uncompress(const std::string& entryPath) const {
  ::ZipEntry entry;
  int32_t status = ::FindEntry(handle_, ::ZipString(entryPath.c_str()), &entry);
  if (status != 0) {
    return nullptr;
  }
  std::unique_ptr<MemoryChunk> chunk = MemoryChunk::Allocate(entry.uncompressed_length);
  status = ::ExtractToMemory(handle_, &entry, chunk->buf, chunk->size);
  if (status != 0) {
    return nullptr;
  }
  return chunk;
}

Result<uint32_t> ZipFile::Crc(const std::string& entryPath) const {
  ::ZipEntry entry;
  int32_t status = ::FindEntry(handle_, ::ZipString(entryPath.c_str()), &entry);
  return status == 0 ? Result<uint32_t>(entry.crc32) : kResultError;
}

}  // namespace idmap2
}  // namespace android
