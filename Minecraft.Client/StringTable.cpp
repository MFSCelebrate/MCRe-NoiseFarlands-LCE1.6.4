#include "stdafx.h"
#include "StringTable.h"

#include "../Minecraft.World/File.h"
#include "../Minecraft.World/StringHelpers.h"

#include <algorithm>
#include <fstream>
#include <sstream>

// Fireblade - heavily modified from original
// Fireblade - switched from locs to xmls

namespace
{
	class XmlStringTableCallback : public ATG::ISAXCallback
	{
	public:
		explicit XmlStringTableCallback(StringTable* table)
			: m_table(table),
			m_insideValue(false)
		{
		}

		HRESULT StartDocument() override { return S_OK; }
		HRESULT EndDocument() override { return S_OK; }

		HRESULT ElementBegin(CONST WCHAR* strName, UINT NameLen, CONST ATG::XMLAttribute* pAttributes, UINT NumAttributes) override
		{
			const wstring elementName(strName, NameLen);

			if (equalsIgnoreCase(elementName, L"data"))
			{
				m_id.clear();
				m_value.clear();

				for (UINT i = 0; i < NumAttributes; ++i)
				{
					const ATG::XMLAttribute& attribute = pAttributes[i];
					const wstring attributeName(attribute.strName, attribute.NameLen);
					if (equalsIgnoreCase(attributeName, L"name") && attribute.strValue != nullptr)
					{
						m_id.assign(attribute.strValue, attribute.ValueLen);
						break;
					}
				}
			}
			else if (equalsIgnoreCase(elementName, L"value") && !m_id.empty())
			{
				m_insideValue = true;
				m_value.clear();
			}

			return S_OK;
		}

		HRESULT ElementContent(CONST WCHAR* strData, UINT DataLen, BOOL /*More*/) override
		{
			if (m_insideValue && strData != nullptr && DataLen > 0)
			{
				m_value.append(strData, DataLen);
			}
			return S_OK;
		}

		HRESULT ElementEnd(CONST WCHAR* strName, UINT NameLen) override
		{
			const wstring elementName(strName, NameLen);

			if (equalsIgnoreCase(elementName, L"value"))
			{
				m_insideValue = false;
			}
			else if (equalsIgnoreCase(elementName, L"data"))
			{
				if (!m_id.empty())
				{
					m_table->setStringValue(m_id, m_value);
				}

				m_id.clear();
				m_value.clear();
				m_insideValue = false;
			}

			return S_OK;
		}

		HRESULT CDATABegin() override { return S_OK; }

		HRESULT CDATAData(CONST WCHAR* strCDATA, UINT CDATALen, BOOL /*bMore*/) override
		{
			if (m_insideValue && strCDATA != nullptr && CDATALen > 0)
			{
				m_value.append(strCDATA, CDATALen);
			}
			return S_OK;
		}

		HRESULT CDATAEnd() override { return S_OK; }

		VOID Error(HRESULT hError, CONST CHAR* strMessage) override
		{
			app.DebugPrintf("String XML parse error (%08X): %s\n", hError, strMessage ? strMessage : "(unknown)");
		}

	private:
		StringTable* m_table;
		bool m_insideValue;
		wstring m_id;
		wstring m_value;
	};

	bool IsXmlFileName(const wstring& path)
	{
		if (path.length() < 4) return false;
		return toLower(path.substr(path.length() - 4)) == L".xml";
	}

	void ReplaceAll(string& target, const string& from, const string& to)
	{
		if (from.empty()) return;
		size_t pos = 0;
		while ((pos = target.find(from, pos)) != string::npos)
		{
			target.replace(pos, from.length(), to);
			pos += to.length();
		}
	}

	string DecodeXmlEntities(string text)
	{
		ReplaceAll(text, "&lt;", "<");
		ReplaceAll(text, "&gt;", ">");
		ReplaceAll(text, "&quot;", "\"");
		ReplaceAll(text, "&apos;", "'");
		ReplaceAll(text, "&amp;", "&");
		return text;
	}

	string ToNativePath(const wstring& path)
	{
		string out(path.begin(), path.end());
#if defined(__PS3__) || defined(__ORBIS__)
		std::replace(out.begin(), out.end(), '\\', '/');
#else
		std::replace(out.begin(), out.end(), '/', '\\');
#endif
		return out;
	}
} // namespace

StringTable::StringTable(void)
{
	isStatic = false;
}

StringTable::StringTable(const wstring& xmlRootPath)
{
	isStatic = true;
	m_xmlRootPath = xmlRootPath;
	ProcessXmlStringTableData(m_xmlRootPath);
}

// Load string table from a binary blob, filling out with the current localisation data only
StringTable::StringTable(PBYTE pbData, DWORD dwSize)
{
	isStatic = false;
	m_xmlRootPath.clear();
	src = byteArray(pbData, dwSize);

	ProcessStringTableData();
}


void StringTable::ReloadStringTable() // Fireblade - dynamically reloads table based off file format used
{
	m_stringsMap.clear();
	m_stringsVec.clear();

	if (!m_xmlRootPath.empty())
	{
		ProcessXmlStringTableData(m_xmlRootPath);
	}
	else
	{
		ProcessStringTableData();
	}
}

void StringTable::setStringValue(const wstring& id, const wstring& value)
{
	if (id.empty()) return;
	m_stringsMap[id] = value;
}

bool StringTable::hasStringKey(const wstring& id) const
{
	if (id.empty()) return false;
	return m_stringsMap.find(id) != m_stringsMap.end();
}

void StringTable::ProcessStringTableData(void)
{
	ByteArrayInputStream bais(src);
	DataInputStream dis(&bais);

	int versionNumber = dis.readInt();
	int languagesCount = dis.readInt();

	vector< pair<wstring, int> > langSizeMap;
	for (int i = 0; i < languagesCount; ++i)
	{
		wstring langId = dis.readUTF();
		int langSize = dis.readInt();

		langSizeMap.push_back(vector< pair<wstring, int> >::value_type(langId, langSize));
	}

	vector<wstring> locales;
	app.getLocale(locales);

	bool foundLang = false;
	int64_t bytesToSkip = 0;
	int dataSize = 0;

	//
	for (auto it_locales = locales.begin();
		it_locales != locales.end() && (!foundLang);
		++it_locales)
	{
		bytesToSkip = 0;

		for (auto& it : langSizeMap)
		{
			if (it.first.compare(*it_locales) == 0)
			{
				app.DebugPrintf("StringTable:: Found language '%ls'.\n", it_locales->c_str());
				dataSize = it.second;
				foundLang = true;
				break;
			}

			bytesToSkip += it.second;
		}

		if (!foundLang)
			app.DebugPrintf("StringTable:: Can't find language '%ls'.\n", it_locales->c_str());
	}

	if (foundLang)
	{
		dis.skip(bytesToSkip);

		byteArray langData(dataSize);
		dis.read(langData);

		dis.close();

		ByteArrayInputStream bais2(langData);
		DataInputStream dis2(&bais2);

		// Read the language file for the selected language
		int langVersion = dis2.readInt();

		isStatic = false;     // 4J-JEV: Versions 1 and up could use
		if (langVersion > 0)  // integers rather than wstrings as keys.
			isStatic = dis2.readBoolean();

		wstring langId = dis2.readUTF();
		int totalStrings = dis2.readInt();

		app.DebugPrintf("IsStatic=%d totalStrings = %d\n", isStatic ? 1 : 0, totalStrings);

		if (!isStatic)
		{
			for (int i = 0; i < totalStrings; ++i)
			{
				wstring stringId = dis2.readUTF();
				wstring stringValue = dis2.readUTF();

				m_stringsMap.insert(unordered_map<wstring, wstring>::value_type(stringId, stringValue));
			}
		}
		else
		{
			for (int i = 0; i < totalStrings; ++i)
				m_stringsVec.push_back(dis2.readUTF());
		}
		dis2.close();

		// We can't delete this data in the dtor, so clear the reference
		bais2.reset();
	}
	else
	{
		app.DebugPrintf("Failed to get language\n");


		isStatic = false;
	}

	// We can't delete this data in the dtor, so clear the reference
	bais.reset();
}

void StringTable::ProcessXmlStringTableData(const wstring& xmlRootPath)
{
	ProcessXmlDirectory(xmlRootPath);

	vector<wstring> locales;
	app.getLocale(locales);

	bool loadedLocaleDirectory = false;
	for (auto& locale : locales)
	{
		const wstring localePath = xmlRootPath + L"\\" + locale;
		File localeFolder(localePath);
		if (localeFolder.exists() && localeFolder.isDirectory())
		{
			app.DebugPrintf("Loading XML locale '%ls'.\n", locale.c_str());
			ProcessXmlDirectory(localePath);
			loadedLocaleDirectory = true;
			break;
		}
	}

	if (!loadedLocaleDirectory)
	{
		app.DebugPrintf("No localization XML found, falling back to .loc file.\n");
	}

	app.DebugPrintf("StringTable:: XML loaded %u keys from '%ls'.\n", static_cast<unsigned>(m_stringsMap.size()), xmlRootPath.c_str());

	isStatic = true;
}

void StringTable::ProcessXmlDirectory(const wstring& directoryPath)
{
	File directory(directoryPath);
	if (!directory.exists() || !directory.isDirectory()) return;

	std::vector<File*>* files = directory.listFiles();
	if (files == nullptr) return;

	vector<wstring> xmlFiles;
	for (auto* file : *files)
	{
		if (file != nullptr)
		{
			if (file->isFile() && IsXmlFileName(file->getName()))
			{
				xmlFiles.push_back(file->getPath());
			}
			delete file;
		}
	}
	delete files;

	sort(xmlFiles.begin(), xmlFiles.end());
	for (auto& xmlFile : xmlFiles)
	{
		ProcessXmlFile(xmlFile);
	}
}

void StringTable::ProcessXmlFile(const wstring& filePath)
{
	ATG::XMLParser parser;
	XmlStringTableCallback callback(this);
	parser.RegisterSAXCallbackInterface(&callback);

	const string nativePath = ToNativePath(filePath);
	const size_t beforeCount = m_stringsMap.size();
	if (FAILED(parser.ParseXMLFile(nativePath.c_str())))
	{
		app.DebugPrintf("StringTable:: Failed to parse XML localization file '%s'.\n", nativePath.c_str());
		ProcessXmlFileLoose(filePath);
		return;
	}

	if (m_stringsMap.size() == beforeCount)
	{
		ProcessXmlFileLoose(filePath);
	}
}

void StringTable::ProcessXmlFileLoose(const wstring& filePath)
{
	const string nativePath = ToNativePath(filePath);
	std::ifstream input(nativePath.c_str(), std::ios::binary);
	if (!input.is_open())
	{
		return;
	}

	std::ostringstream contents;
	contents << input.rdbuf();
	string xml = contents.str();

	int parsedCount = 0;
	size_t cursor = 0;
	while (true)
	{
		const size_t dataStart = xml.find("<data", cursor);
		if (dataStart == string::npos) break;

		const size_t tagEnd = xml.find('>', dataStart);
		if (tagEnd == string::npos) break;

		size_t namePos = xml.find("name=\"", dataStart);
		if (namePos == string::npos || namePos > tagEnd)
		{
			cursor = tagEnd + 1;
			continue;
		}
		namePos += 6;
		const size_t nameEnd = xml.find('"', namePos);
		if (nameEnd == string::npos)
		{
			cursor = tagEnd + 1;
			continue;
		}

		const size_t valueOpen = xml.find("<value>", tagEnd);
		const size_t dataClose = xml.find("</data>", tagEnd);
		if (valueOpen == string::npos || dataClose == string::npos || valueOpen > dataClose)
		{
			cursor = tagEnd + 1;
			continue;
		}

		const size_t valueStart = valueOpen + 7;
		const size_t valueEnd = xml.find("</value>", valueStart);
		if (valueEnd == string::npos || valueEnd > dataClose)
		{
			cursor = dataClose + 7;
			continue;
		}

		string id = xml.substr(namePos, nameEnd - namePos);
		string value = xml.substr(valueStart, valueEnd - valueStart);
		value = DecodeXmlEntities(value);

		setStringValue(convStringToWstring(id), convStringToWstring(value));
		parsedCount++;

		cursor = dataClose + 7;
	}

}
StringTable::~StringTable(void)
{
	// delete src.data; TODO 4J-JEV: ?
}

void StringTable::getData(PBYTE* ppData, UINT* pSize)
{
	*ppData = src.data;
	*pSize = src.length;
}

LPCWSTR StringTable::getString(const wstring& id)
{
	auto it = m_stringsMap.find(id);

	if (it != m_stringsMap.end())
	{
		return it->second.c_str();
	}
	else
	{
		m_missingKeyFallback = id;
		return m_missingKeyFallback.c_str();
	}
}

LPCWSTR StringTable::getString(int id)
{
	if (id >= 0 && static_cast<size_t>(id) < m_stringsVec.size())
	{
		LPCWSTR pwchString = m_stringsVec.at(id).c_str();
		return pwchString;
	}
	else
		return L"";
}