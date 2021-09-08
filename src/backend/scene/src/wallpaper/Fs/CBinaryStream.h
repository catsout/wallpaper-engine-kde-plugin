#pragma once

#include <cstdio>
#include <string>
#include <string_view>
#include <memory>
#include <filesystem>
#include "IBinaryStream.h"
#include "../Utils/Logging.h"

namespace wallpaper 
{
namespace fs
{
class CBinarayStream : public IBinaryStream {
public:
	virtual ~CBinarayStream() {
		std::fclose(m_file);
	}
protected:
	CBinarayStream(std::string_view path, std::FILE* file):m_file(file) {}
public:
    virtual size_t Read(void* buffer, size_t sizeInBytes) override {
		return sizeInBytes * std::fread(buffer, sizeInBytes, 1, m_file);
	}
    virtual size_t Write( const void* buffer, size_t sizeInBytes) override { return 0; }
    virtual char* Gets( char* buffer, size_t sizeStr) override {
		return std::fgets(buffer, sizeStr, m_file);
	}
	virtual long Tell() const override {
		return std::ftell(m_file);
	}
	virtual bool SeekSet(long offset) override {
		return std::fseek(m_file, offset, SEEK_SET) == 0;
	}
	virtual bool SeekCur(long offset) override {
		return std::fseek(m_file, offset, SEEK_CUR) == 0;
	}
	virtual bool SeekEnd(long offset) override {
		return std::fseek(m_file, offset, SEEK_END) == 0;
	}
    virtual std::size_t Size() const override {
		long cur = std::ftell(m_file);
		std::fseek(m_file, 0, SEEK_END);
		long size = std::ftell(m_file);
		std::fseek(m_file, cur, SEEK_SET); // seek back
		return size;
	}
private:
	std::string m_path;
	std::FILE* m_file;
};

inline std::shared_ptr<CBinarayStream> CreateCBinaryStream(std::string_view path) {
	struct Shared : public CBinarayStream {
		Shared(std::string_view path, std::FILE* file):CBinarayStream(path, file) {};
	};
	if(std::filesystem::is_directory(path)) {
		LOG_ERROR("can't open: %s", path.data());
	 	return nullptr;
	}
	auto* file = std::fopen(std::string(path).c_str(), "rb");
	if(file == NULL) {
		LOG_ERROR("can't open: %s", path.data());
		return nullptr;
	}
	auto cb = std::make_shared<Shared>(path, file);
	return cb;
}
}
}