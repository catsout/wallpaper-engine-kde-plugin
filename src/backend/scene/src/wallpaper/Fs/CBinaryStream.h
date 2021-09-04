#pragma once

#include <cstdio>
#include <string>
#include <string_view>
#include <memory>
#include <filesystem>
#include "IBinaryStream.h"
#include "../Utils/Log.h"

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
    virtual int Read(void* buffer, int sizeInBytes) override {
		return std::fread(buffer, sizeInBytes, 1, m_file);
	}
    virtual int Write( const void* buffer, int sizeInBytes) override { return 0; }
    virtual char* Gets( char* buffer, int sizeStr) override {
		return std::fgets(buffer, sizeStr, m_file);
	}
	virtual long Tell() override {
		return std::ftell(m_file);
	}
	virtual bool SeekSet(long offset) override {
		return std::fseek(m_file, offset, SEEK_SET);
	}
	virtual bool SeekCur(long offset) override {
		return std::fseek(m_file, offset, SEEK_CUR);
	}
	virtual bool SeekEnd(long offset) override {
		return std::fseek(m_file, offset, SEEK_END);
	}
    virtual std::size_t Size() override {
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
		LOG_ERROR("can't open " + std::string(path));
	 	return nullptr;
	}
	auto* file = std::fopen(std::string(path).c_str(), "rb");
	if(file == NULL) {
		LOG_ERROR("can't open " + std::string(path));
		return nullptr;
	}
	auto cb = std::make_shared<Shared>(path, file);
	return cb;
}
}
}