# What is ZipReader?
ZipReader is a minimal zip file central directory parser written for C++ projects.  Instead of needing to lean on platform specific solutions (such as IShellDispatch objects in Windows) to retrieve the contents of a zip file, ZipReader parses the central directory and retrieves the directory entries __without the need for extraction__.  Skipping the extraction process (which happens behind the scenes when using solutions such as IShellDispatch objects) provides a major boost in performance and greatly reduces runtime when dealing with larger (or multiple) zip files.

# Future Plans
At this time, ZipReader is released with the expectation that it will be extended to fit the needs of specific projects.  Although it is currently packaged as a DLL, it is simple enough to be easily integrated as a regular class (or even a single header file if desired) into a larger project.  Aside from bug fixes, no further enhancements are planned at this time.

# Example Usage
ZipReader can be used in the following way:
```c++
ZipReader zr;
	
if (zr.SetInput("blah.zip") == ZipReaderStatus::S_FAIL)
		std::cout << "set input error" << std::endl;

std::vector<std::string> entries;
if (zr.GetEntries(entries) == ZipReaderStatus::S_FAIL)
		std::cout << "get entries error" << std::endl;

for (auto entry : entries)
		std::cout << entry << std::endl;
```
