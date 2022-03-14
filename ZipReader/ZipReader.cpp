#include "ZipReader.h"

#include <stdexcept>
#include <iostream>
#include <assert.h>

ZipReader::ZipReader()
{
}

ZipReader::~ZipReader()
{
}

ZipReaderStatus ZipReader::GetEntries(std::vector<std::string> &entries)
{
	// This could happen if SetEntries() was never called
	if (!m_ifs.is_open())
		return ZipReaderStatus::S_FAIL;

	try {
		// Main loop
		PositionAtCDEnd();

		PositionAtCDStart();

		while (!m_ifs.eof()) {
			ReadEntry(entries);
			if (!FindNextEntry())
				break;
		}

		return ZipReaderStatus::S_OK;
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return ZipReaderStatus::S_FAIL;
	}
}

ZipReaderStatus ZipReader::SetInput(std::string file)
{
	try {
		ValidateInput(file);

		// Create the ifstream object
		m_ifs = std::ifstream(file, std::ios::ate | std::ios::binary);
		if (!m_ifs.is_open())
			throw std::runtime_error("Ifstream is null!");

		// Set the file size
		SetFileSize();

		return ZipReaderStatus::S_OK;
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
		return ZipReaderStatus::S_FAIL;
	}
}

void ZipReader::ValidateInput(std::string file)
{
	// Check for existence of extension
	size_t pos = file.rfind('.');
	if (pos == std::string::npos)
		throw std::invalid_argument("File extension not found!");

	// Validate the extension
	std::string ext = file.substr(pos + 1, file.length());
	if (ext != ZIPEXT)
		throw std::invalid_argument("File is not in .zip format!");
}

void ZipReader::SetFileSize()
{
	m_fileSize = m_ifs.tellg();
}

void ZipReader::PositionAtCDEnd()
{
	int pos = m_fileSize - sizeof(unsigned int);
	int seek_off = 0;

	unsigned int val = 0;
	while (pos > 0) {
		// Set the seek position
		seek_off = -(m_fileSize - pos);
		m_ifs.seekg(seek_off, std::ios::end);

		// Read an unsigned int from the file
		m_ifs.read((char *)&val, sizeof(val));

		// Check if we've found the end of a central directory entry
		if (val == CD_END) {
			// Back the seek up 4 bytes
			int segmentSize = sizeof(val);
			m_ifs.seekg(-segmentSize, std::ios::cur);
			return;
		}

		// Move back one byte and grab another unsigned int
		pos--;
	}
}

void ZipReader::PositionAtCDStart()
{
	unsigned int val = 0;

	// Get the CD start by jumping to the offset in the EOCDR
	m_ifs.seekg(CD_OFF_START, std::ios::cur);
	m_ifs.read((char *)&val, sizeof(val));

	// Position at CD start by using the offset located in the EOCDR
	m_ifs.seekg(val, std::ios::beg);
}

void ZipReader::ReadEntry(std::vector<std::string> &entries)
{
	// Save the cd start position of the current entry
	int cdStart = m_ifs.tellg();
	// Move to the beginning of the central directory + offset of 46 to get the filename
	m_ifs.seekg(CD_OFF_FN, std::ios::cur);
	int endOfEntry = GetEndOfEntry();

	// Break on no end of entry found - should never happen
	assert(endOfEntry);

	int entrySize = endOfEntry - (cdStart + CD_OFF_FN) - 1;
	// Add an extra byte for the null terminator
	char *entry = new char[entrySize + 1];
	m_ifs.read(entry, entrySize);
	entry[entrySize] = '\0';

	entries.push_back(entry);
	delete[] entry;
}

bool ZipReader::FindNextEntry()
{
	int curPos = m_ifs.tellg();

	unsigned int header = 0;
	while (header != CD_START) {
		m_ifs.read((char *)&header, sizeof(header));
		m_ifs.seekg(++curPos, std::ios::beg);

		// Break on end of file
		if (curPos > m_fileSize) {
			return false;
		}
	}

	// Set the seek position to the next found record
	m_ifs.seekg(curPos - 1, std::ios::beg);
	return true;
}

int ZipReader::GetEndOfEntry()
{
	int curPos = m_ifs.tellg();

	int tempPos = curPos;
	unsigned int header = 0;
	while (header != CD_START && header != CD_END) {
		m_ifs.read((char *)&header, sizeof(header));
		m_ifs.seekg(++tempPos, std::ios::beg);

		// Break on end of file - should never happen
		if (tempPos > m_fileSize) {
			throw std::runtime_error("End of entry or CD end never found!");
		}
	}

	// Reset the seek position
	m_ifs.seekg(curPos, std::ios::beg);

	return tempPos;
}