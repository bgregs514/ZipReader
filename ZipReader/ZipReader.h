#pragma once
#ifdef MAKEZIPREADER
#	define DLL_API __declspec(dllexport)
#else
#	define DLL_API __declspec(dllimport)
#endif

#include <fstream>
#include <vector>
#include <string>

enum class ZipReaderStatus {
	S_OK,
	S_FAIL
};

/* The main ZipReader object that handles the parsing of zip files in binary format */
class DLL_API ZipReader
{
	/*
	* ZipReader prases a zip file's central directory using the following logic:
	*	1) Find the end of the central directory record (EOCDR) by scanning the zip file in binary format
	*		for the EOCDR signature, beginning at the end of the file
	*	2) Use the offset located in the EOCDR to position the stream reader at the beginning of the central
	*		directory
	*	3) Use the offset to position the stream reader at the file name and track its position start point
	*	4) Scan until either another central directory header is found or the EOCDR is found and track the position
	*	5) Reset the reader to the start of the file name and read until the end
	*	6) Position the reader over the next header or terminate if the EOCDR is found
	*/
public:
	ZipReader();
	~ZipReader();

	ZipReaderStatus GetEntries(std::vector<std::string> &entries);
	ZipReaderStatus SetInput(std::string file);

private:
	/*************************
	* METHODS
	*************************/
	// Validations
	void ValidateInput(std::string file);

	// Core logic flow
	void PositionAtCDEnd();
	void PositionAtCDStart();
	void ReadEntry(std::vector<std::string> &entries);
	bool FindNextEntry();

	// Helper methods
	int GetEndOfEntry();
	void SetFileSize();

	/*************************
	* VARIABLES
	*************************/
	std::ifstream m_ifs;
	int m_fileSize = 0;

	// Constants
	static constexpr char ZIPEXT[] = "zip";
	static constexpr int CD_START	= 0x02014b50;
	static constexpr int CD_END	= 0x06054b50;
	static constexpr int CD_OFF_FN	= 46;
	static constexpr int CD_OFF_START = 16;
};