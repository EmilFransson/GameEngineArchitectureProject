#include "pch.h"
#include "ResourceManager.h"
#include "PackageTool.h"
#include "OBJ_Loader.h"
#include "UI.h"

/* Include low level I/O */
#include <io.h>
#include <fcntl.h>

ResourceManager* ResourceManager::s_Instance;
std::vector<std::thread> ResourceManager::m_tWorkers;
size_t ResourceManager::m_numThreads;
std::mutex ResourceManager::m_tQueueMutex;
std::condition_variable ResourceManager::m_tCondition;
bool ResourceManager::m_tTerminate;
std::deque<ResourceManager::JobHolder*> ResourceManager::m_tQueue;
std::unique_ptr<PoolAllocator<Texture2D>> ResourceManager::m_pTextureAllocator = std::make_unique<PoolAllocator<Texture2D>>("TextureAllocator", 6);
std::unique_ptr<PoolAllocator<MeshOBJ>> ResourceManager::m_pMeshOBJAllocator = std::make_unique<PoolAllocator<MeshOBJ>>("MeshOBJAllocator", 87);
std::unique_ptr<PoolAllocator<Material>> ResourceManager::m_pMaterialAllocator = std::make_unique<PoolAllocator<Material>>("MaterialAllocator", 100);
BuddyAllocator ResourceManager::buddyAllocator = BuddyAllocator();
ResourceManager::BuddyFree ResourceManager::buddyFree = {};
std::mutex ResourceManager::m_GuidToResourceMutex;
std::map<std::string, std::mutex> ResourceManager::m_FilenameToMutexMap;

void ResourceManager::Init()
{
	if (!s_Instance)
	{
		s_Instance = DBG_NEW ResourceManager();
	}
	m_tTerminate = false;
	m_numThreads = std::thread::hardware_concurrency();
	m_tWorkers.reserve(m_numThreads);
	for (size_t i = 0; i < m_numThreads; i++)
	{
		//Maybe rework?
		m_tWorkers.push_back(std::thread(
			tWaitForJob
		));
	}
}

void ResourceManager::CleanUp()
{
	tShutdown();

	delete s_Instance;
}

void ResourceManager::FreeMemory() noexcept
{
	std::vector<std::pair<uint64_t, uint64_t>> keys;
	for (auto it = m_GUIDToResourceMap.begin(); it != m_GUIDToResourceMap.end(); it++)
	{
		if (it->second.use_count() == 1 && (it->second->GetName() != "Cube.obj" || it->second->GetName() != "Grey.png"))
		{
			std::cout << "Unncecessary resource(s) removed\n";
			keys.push_back(it->first);
		}
	}
	for (auto& key : keys)
	{
		if (m_GUIDToResourceMap[key]->GetType() == "TEX")
		{
			std::lock_guard<std::mutex> lock(m_GUIDToMutexMap[key]);
			m_GUIDToResourceMap.erase(key);
		}
		else if (m_GUIDToResourceMap[key]->GetType() == "MESH")
		{
			std::lock_guard<std::mutex> lock(m_FilenameToMutexMap[dynamic_pointer_cast<MeshOBJ>(m_GUIDToResourceMap[key])->m_FileName]);
			m_GUIDToResourceMap.erase(key);
		}
		else
		{
			m_GUIDToResourceMap.erase(key);
		}
	}
}

void ResourceManager::DisplayStateUI()
{
	std::vector<std::pair<std::string, long>> textures;
	std::vector<std::pair<std::string, long>> meshes;
	std::vector<std::pair<std::string, long>> materials;

	std::unique_lock<std::mutex> lock(m_GuidToResourceMutex);
	for (auto it = s_Instance->m_GUIDToResourceMap.begin(); it != s_Instance->m_GUIDToResourceMap.end(); ++it)
	{
		std::pair<std::string, long> pair;
		pair.first = it->second->GetName();
		pair.second = it->second.use_count() - 1;
		if (it->second->GetType() == "TEX")
		{
			textures.push_back(pair);
		}
		else if (it->second->GetType() == "MESH")
		{
			meshes.push_back(pair);
		}
		else // is of type mat
		{
			materials.push_back(pair);
		}
	}

	static bool texturePressed = true;
	static bool meshPressed = true;
	static bool materialPressed = true;
	ImGui::Begin("Resource Manager");
	ImGui::Checkbox("Textures", &texturePressed);
	ImGui::SameLine();
	ImGui::Checkbox("Meshes", &meshPressed);
	ImGui::SameLine();
	ImGui::Checkbox("Materials", &materialPressed);

	if (texturePressed == true && !textures.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("TEXTURES:");
		ImGui::PopStyleColor();
		for (auto& texture : textures)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			ImGui::Text(texture.first.c_str());
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::Text(":");
			ImGui::Text("Resourcetype: Texture");
			ImGui::Text("References:");
			ImGui::SameLine();
			ImGui::Text("%d", texture.second);
		}
	}
	
	if (meshPressed == true && !meshes.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("MESHES:");
		ImGui::PopStyleColor();
		for (auto& mesh : meshes)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			ImGui::Text(mesh.first.c_str());
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::Text(":");
			ImGui::Text("Resourcetype: Mesh");
			ImGui::Text("References:");
			ImGui::SameLine();
			ImGui::Text("%d", mesh.second);
		}
	}
	if (materialPressed == true && !materials.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("MATERIALS:");
		ImGui::PopStyleColor();
		for (auto& material : materials)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			ImGui::Text(material.first.c_str());
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::Text(":");
			ImGui::Text("Resourcetype: Mesh");
			ImGui::Text("References:");
			ImGui::SameLine();
			ImGui::Text("%d", material.second);
		}
	}
	ImGui::End();
}

template<>
std::shared_ptr<Texture2D> ResourceManager::Load(const std::pair<uint64_t, uint64_t>& guid) noexcept
{
	bool result = true;
	{
		std::unique_lock<std::mutex> lock(m_GUIDToMutexMap[guid]);
		if (m_GUIDToResourceMap.contains(guid))
		{
			return dynamic_pointer_cast<Texture2D>(m_GUIDToResourceMap[guid]);
		}
		else
		{
			if (m_GUIDToPackageMap.contains(guid))
			{
				std::unique_lock<std::mutex> lock2(m_GuidToResourceMutex);
				result = LoadResourceFromPackage(guid);
				if (result == true)
				{
					return dynamic_pointer_cast<Texture2D>(m_GUIDToResourceMap[guid]);
				}
			}
		}
	}

	if ((!m_GUIDToResourceMap.contains(guid) && !m_GUIDToPackageMap.contains(guid)) || !result)
	{
		std::unique_lock<std::mutex> lock(m_GUIDToMutexMap[ConvertGUIDToPair(m_FileNameToGUIDMap["Grey.png"])]);

		std::cout << "Error: Unable to load asset " << "Placeholder loaded instead.\n";
		if (m_GUIDToResourceMap.contains(ConvertGUIDToPair(m_FileNameToGUIDMap["Grey.png"])))
		{
			return dynamic_pointer_cast<Texture2D>(m_GUIDToResourceMap[ConvertGUIDToPair(m_FileNameToGUIDMap["Grey.png"])]);
		}
		else
		{
			std::unique_lock<std::mutex> lock2(m_GuidToResourceMutex);
			if (LoadResourceFromPackage(ConvertGUIDToPair(m_FileNameToGUIDMap["Grey.png"])))
			{
				return dynamic_pointer_cast<Texture2D>(m_GUIDToResourceMap[ConvertGUIDToPair(m_FileNameToGUIDMap["Grey.png"])]);
			}
		}
	}
	return nullptr; //Should never be reached.
}

template<>
std::shared_ptr<MeshOBJ> ResourceManager::Load(const std::pair<uint64_t, uint64_t>& guid) noexcept
{
	int package_fd;
	std::string filePath = "Packages/" + std::string(m_GUIDToPackageMap[guid]);
	errno_t err = _sopen_s(&package_fd, filePath.c_str(), _O_BINARY | _O_RDONLY, _SH_DENYWR, _S_IREAD);
	if (err == 0)
	{
		PackageTool::PackageHeader packageHeader{};
		int bytes_read = _read(package_fd, (char*)&packageHeader, sizeof(PackageTool::PackageHeader));
		bool foundOBJ = false;
		for (uint32_t i{0u}; i < packageHeader.assetCount && foundOBJ == false; ++i)
		{
			PackageTool::ChunkHeader chunkHeader{};
			bytes_read = _read(package_fd, (char*)&chunkHeader, sizeof(PackageTool::ChunkHeader));

			auto objName = std::unique_ptr<char, BuddyFree>(buddyAllocator.calloc<char>(chunkHeader.readableSize + 1), buddyFree);
			bytes_read = _read(package_fd, objName.get(), static_cast<unsigned int>(chunkHeader.readableSize));
			//_lseek(package_fd, static_cast<long>(chunkHeader.readableSize), SEEK_CUR); // Skip readable name

			if (memcmp(chunkHeader.type, "MESH", 4) == 0)
			{
				PackageTool::MeshHeader meshHeader{};
				bytes_read = _read(package_fd, (char*)&meshHeader, sizeof(PackageTool::MeshHeader));
				if (ConvertGUIDToPair(chunkHeader.guid) == guid)
				{
					std::vector<objl::Vertex> vertices;
					vertices.resize(meshHeader.verticesDataSize / sizeof(objl::Vertex));
					bytes_read = _read(package_fd, (char*)vertices.data(), static_cast<unsigned int>(meshHeader.verticesDataSize));
					std::vector<unsigned int> indices;
					indices.resize(meshHeader.indicesDataSize / sizeof(unsigned int));
					bytes_read = _read(package_fd, (char*)indices.data(), static_cast<unsigned int>(meshHeader.indicesDataSize));

					if (m_pMeshOBJAllocator->GetByteUsage() + sizeof(MeshOBJ) > m_pMeshOBJAllocator->GetByteCapacity())
					{
						std::cout << "WARNING: Approaching max byte size limit. Attempting to free unused assets.\n";
						FreeMemory();
						if (m_pMeshOBJAllocator->GetByteUsage() + sizeof(MeshOBJ) > m_pMeshOBJAllocator->GetByteCapacity())
						{
							std::cout << "Error: Cannot create resource. Unable to free enough memory.\n";
							return nullptr;
						}
						
					}
					if (strcmp(meshHeader.materialName, "") != 0)
					{
						std::unique_lock<std::mutex> lock(m_GuidToResourceMutex);
						m_GUIDToResourceMap[guid] = dynamic_pointer_cast<Resource>(std::shared_ptr<MeshOBJ>(m_pMeshOBJAllocator->New(vertices,
																				   indices, 
																				   Load<Material>(ConvertGUIDToPair(m_FileNameToGUIDMap[meshHeader.materialName])),
																				   meshHeader.meshName, objName.get(), "MESH"), [](MeshOBJ* pData) {
																				   m_pMeshOBJAllocator->Delete(pData);
																					}));
					}
					else
					{
						std::unique_lock<std::mutex> lock(m_GuidToResourceMutex);
						m_GUIDToResourceMap[guid] = dynamic_pointer_cast<Resource>(std::shared_ptr<MeshOBJ>(m_pMeshOBJAllocator->New(vertices, indices, nullptr, meshHeader.meshName, objName.get(), "MESH"),
																				   [](MeshOBJ* pData){
																				   m_pMeshOBJAllocator->Delete(pData);
																					}));
					}
					return dynamic_pointer_cast<MeshOBJ>(m_GUIDToResourceMap[guid]);
				}
				else
				{
					_lseek(package_fd, chunkHeader.chunkSize - sizeof(PackageTool::MeshHeader), SEEK_CUR);
				}
			}
			else
			{
				_lseek(package_fd, chunkHeader.chunkSize, SEEK_CUR);
			}
		}
		_close(package_fd);
	}
	else
	{
		//...
	}
	return nullptr;
}

template<>
std::shared_ptr<Material> ResourceManager::Load(const std::pair<uint64_t, uint64_t>& guid) noexcept
{
	if (m_GUIDToResourceMap.contains(guid))
	{
		return dynamic_pointer_cast<Material>(m_GUIDToResourceMap[guid]);
	}
	else
	{
		int package_fd;
		std::string filePath = "Packages/" + std::string(m_GUIDToPackageMap[guid]);
		errno_t err = _sopen_s(&package_fd, filePath.c_str(), _O_BINARY | _O_RDONLY, _SH_DENYWR, _S_IREAD);
		if (err == 0)
		{
			PackageTool::PackageHeader packageHeader{};
			int bytes_read = _read(package_fd, (char*)&packageHeader, sizeof(PackageTool::PackageHeader));
			bool foundMaterial = false;
			PackageTool::MaterialHeader materialHeader{};
			for (uint32_t i{ 0u }; i < packageHeader.assetCount && foundMaterial == false; ++i)
			{
				PackageTool::ChunkHeader chunkHeader{};
				bytes_read = _read(package_fd, (char*)&chunkHeader, sizeof(PackageTool::ChunkHeader));
				_lseek(package_fd, static_cast<long>(chunkHeader.readableSize), SEEK_CUR); // Skip readable name
				if (memcmp(chunkHeader.type, "MAT", 3) == 0)
				{
					bytes_read = _read(package_fd, (char*)&materialHeader, sizeof(PackageTool::MaterialHeader));
					if (ConvertGUIDToPair(chunkHeader.guid) == guid)
					{
						foundMaterial = true;
					}
					else
					{
						_lseek(package_fd, chunkHeader.chunkSize - sizeof(PackageTool::MaterialHeader), SEEK_CUR);
					}
				}
				else
				{
					_lseek(package_fd, chunkHeader.chunkSize, SEEK_CUR);
				}
			}
			if (foundMaterial)
			{
				if (m_pMaterialAllocator->GetByteUsage() + sizeof(Material) > m_pMaterialAllocator->GetByteCapacity())
				{
					std::cout << "WARNING: Approaching max byte size limit. Attempting to free unused assets.\n";
					FreeMemory();
					if (m_pMaterialAllocator->GetByteUsage() + sizeof(Material) > m_pMaterialAllocator->GetByteCapacity())
					{
						std::cout << "Error: Cannot create resource. Unable to free enough memory.\n";
						return nullptr;
					}
				}
				PackageTool::SMaterial material{};
				bytes_read = _read(package_fd, (char*)&material, materialHeader.dataSize);
				m_GUIDToResourceMap[guid] = dynamic_pointer_cast<Resource>(std::shared_ptr<Material>(m_pMaterialAllocator->New(material, materialHeader.materialName, "MAT"),
																								     [](Material* pData) {
																									 m_pMaterialAllocator->Delete(pData);
																												}));
				return dynamic_pointer_cast<Material>(m_GUIDToResourceMap[guid]);
			}
		}
	}
	return nullptr; //Should never be reached.
}

const bool ResourceManager::LoadResourceFromPackage(const std::pair<uint64_t, uint64_t>& guid) noexcept
{
	int package_fd;
	std::string filePath = "Packages/";
	filePath += std::string(m_GUIDToPackageMap[guid]);
	errno_t err = _sopen_s(&package_fd, filePath.c_str(), _O_BINARY | _O_RDONLY, _SH_DENYWR, _S_IREAD);
	if(err == 0)
	{
		PackageTool::PackageHeader packageHeader{};
		int bytes_read = _read(package_fd, (char*)&packageHeader, sizeof(PackageTool::PackageHeader));
		bool foundAsset = false;
		PackageTool::ChunkHeader chunkHeader{};
		std::unique_ptr<char, BuddyFree> pAssetFileName;
		for (uint32_t i{ 0u }; i < packageHeader.assetCount && foundAsset == false; ++i)
		{
			bytes_read = _read(package_fd, (char*)&chunkHeader, sizeof(PackageTool::ChunkHeader));
			pAssetFileName = std::unique_ptr<char, BuddyFree>(buddyAllocator.calloc<char>(chunkHeader.readableSize + 1), buddyFree);
			bytes_read = _read(package_fd, pAssetFileName.get(), static_cast<unsigned int>(chunkHeader.readableSize));
			if (ConvertGUIDToPair(chunkHeader.guid) == guid)
			{
				foundAsset = true;
			}
			else
			{
				_lseek(package_fd, chunkHeader.chunkSize, SEEK_CUR);
			}
		}
		if (memcmp(chunkHeader.type, "TEX", 3) == 0) //Asset is a texture and should be loaded as such.
		{
			PackageTool::TextureHeader textureHeader{};
			bytes_read = _read(package_fd, (char*)&textureHeader, sizeof(PackageTool::TextureHeader));
			auto textureBuffer = std::unique_ptr<char, BuddyFree>(buddyAllocator.alloc<char>(textureHeader.dataSize), buddyFree);
			bytes_read = _read(package_fd, textureBuffer.get(), textureHeader.dataSize);

			if (m_pTextureAllocator->GetByteUsage() + sizeof(Texture2D) > m_pTextureAllocator->GetByteCapacity())
			{
				std::cout << "WARNING: Approaching max byte size limit. Attempting to free unused assets.\n";
				FreeMemory();
				if (m_pTextureAllocator->GetByteUsage() + sizeof(Texture2D) > m_pTextureAllocator->GetByteCapacity())
				{
					std::cout << "Error: Cannot create resource. Unable to free enough memory.\n";
					return false;
				}
			}
			if (memcmp(textureHeader.textureType, "NORM", 4) == 0) //Normal uncompressed texture type:
			{
				m_GUIDToResourceMap[guid] = dynamic_pointer_cast<Resource>(std::shared_ptr<Texture2D>(m_pTextureAllocator->New(textureHeader.width,
																									  textureHeader.height,
																									  textureHeader.rowPitch,
																									  textureBuffer.get(),
																									  DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM,
																									  pAssetFileName.get(), "TEX"), [](Texture2D* pData) {
																									  	m_pTextureAllocator->Delete(pData);
																									  }));
			}
			else //Compressed texture type:
			{
				m_GUIDToResourceMap[guid] = dynamic_pointer_cast<Resource>(std::shared_ptr<Texture2D>(m_pTextureAllocator->New(textureHeader.width,
																									  textureHeader.height,
																									  textureHeader.rowPitch,
																									  textureBuffer.get(),
																									  DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB,
																									  pAssetFileName.get(), "TEX"), [](Texture2D* pData) {
																									  	m_pTextureAllocator->Delete(pData);
																									  }));
			}
		}
		_close(package_fd);
		return true;
	}
	return false;
}

void ResourceManager::MapPackageContent() noexcept
{
	std::vector<std::filesystem::directory_entry> packages;
	for (const auto& package : std::filesystem::directory_iterator("Packages/"))
	{
		int package_fd;
		std::string packagePath = "Packages/" + package.path().filename().string();
		errno_t err = _sopen_s(&package_fd, packagePath.c_str(), _O_BINARY | _O_RDONLY, _SH_DENYWR, _S_IREAD);
		if (err == 0)
		{
			PackageTool::PackageHeader pkgHdr{};
			int bytes_read = _read(package_fd, (char*)&pkgHdr, sizeof(PackageTool::PackageHeader));
			for (uint32_t i{ 0u }; i < pkgHdr.assetCount; ++i)
			{
				PackageTool::ChunkHeader chkHdr{};
				bytes_read = _read(package_fd, (char*)&chkHdr, sizeof(PackageTool::ChunkHeader));
				auto fileName = std::unique_ptr<char, BuddyFree>(buddyAllocator.calloc<char>(chkHdr.readableSize+1), buddyFree);
				bytes_read = _read(package_fd, fileName.get(), static_cast<unsigned int>(chkHdr.readableSize));
				if (memcmp(chkHdr.type, "MESH", 4) == 0)
				{
					//Find the correct mesh name:
					PackageTool::MeshHeader meshHeader{};
					bytes_read = _read(package_fd, (char*)&meshHeader, sizeof(PackageTool::MeshHeader));

					m_GUIDToPackageMap[ConvertGUIDToPair(chkHdr.guid)] = package.path().filename().string();
					m_FileNameToGUIDMap[meshHeader.meshName] = chkHdr.guid;
					m_OBJToMeshesMap[fileName.get()].push_back(meshHeader.meshName);
					if (i != pkgHdr.assetCount - 1u)
						_lseek(package_fd, chkHdr.chunkSize - sizeof(PackageTool::MeshHeader), SEEK_CUR);
				}
				else if (memcmp(chkHdr.type, "MAT", 3) == 0)
				{
					PackageTool::MaterialHeader materialHeader{};
					bytes_read = _read(package_fd, (char*)&materialHeader, sizeof(PackageTool::MaterialHeader));
					m_GUIDToPackageMap[ConvertGUIDToPair(chkHdr.guid)] = package.path().filename().string();
					m_FileNameToGUIDMap[materialHeader.materialName] = chkHdr.guid;
					if (i != pkgHdr.assetCount - 1u)
						_lseek(package_fd, chkHdr.chunkSize - sizeof(PackageTool::MaterialHeader), SEEK_CUR);
				}
				else
				{
					m_GUIDToPackageMap[ConvertGUIDToPair(chkHdr.guid)] = package.path().filename().string();
					m_FileNameToGUIDMap[fileName.get()] = chkHdr.guid;
					if (i != pkgHdr.assetCount - 1u)
						_lseek(package_fd, chkHdr.chunkSize, SEEK_CUR);
				}
			}
			_close(package_fd);
		}
	}

	//Load place holder objects:
	Load<Texture2D>(ConvertGUIDToPair(m_FileNameToGUIDMap["Grey.png"]));
	Load<MeshOBJ>(ConvertGUIDToPair(m_FileNameToGUIDMap["Cube.obj"]));
}

std::pair<uint64_t, uint64_t> ResourceManager::ConvertGUIDToPair(const GUID& guid) noexcept
{
	auto split = (uint64_t*)&guid;
	return std::make_pair(split[0], split[1]);
}

GUID ResourceManager::ConvertPairToGUID(const std::pair<uint64_t, uint64_t> pGuid) noexcept
{
	auto guid = *(GUID*)&pGuid;
	
	return guid;
}

template<>
std::vector<std::shared_ptr<MeshOBJ>> ResourceManager::LoadMultiple(std::string& objName) noexcept
{
	std::vector<std::string> meshNames;
	//1 Retrieve the list of all mesh names:
	if (m_OBJToMeshesMap.contains(objName))
	{
		meshNames = m_OBJToMeshesMap[objName];
	}
	else
	{
		//Instead add placeholder.
		std::cout << "Error: Unable to load asset " << objName << ". Placeholder loaded instead.\n";
		meshNames = m_OBJToMeshesMap["Cube.obj"];
		objName = "Cube.obj";
	}

	//Prepare the std::vector to be returned:
	std::vector<std::shared_ptr<MeshOBJ>> meshes;

	//Have a std::vector containing flags for whether a certain mesh has been loaded or not
	std::vector<bool> meshesLoadedFlags(meshNames.size(), 0);

	std::unique_lock<std::mutex> lock(m_FilenameToMutexMap[objName]);
	//Load all meshes directly from cache that has already been loaded from the package:
	for (uint32_t i{0u}; i < meshNames.size(); ++i)
	{
		if (m_GUIDToResourceMap.contains(ConvertGUIDToPair(m_FileNameToGUIDMap[meshNames[i]])))
		{
			meshes.push_back(dynamic_pointer_cast<MeshOBJ>(m_GUIDToResourceMap[ConvertGUIDToPair(m_FileNameToGUIDMap[meshNames[i]])]));
			meshesLoadedFlags[i] = true;
		}
	}

	//Load the meshes from package that has not been loaded into cache:
	for (uint32_t i{ 0u }; i < meshNames.size(); i++)
	{
		if (meshesLoadedFlags[i] == false)
		{
			//This mesh has not been loaded from cache (since it didn't exist) and must now be loaded from the package:
			std::shared_ptr<MeshOBJ> pMesh = Load<MeshOBJ>(ConvertGUIDToPair(m_FileNameToGUIDMap[meshNames[i]]));
			if (pMesh == nullptr)
			{
				std::unique_lock<std::mutex> lock2(m_GUIDToMutexMap[ConvertGUIDToPair(m_FileNameToGUIDMap["Cube"])]);

				std::cout << "Error: Unable to load asset " << "Placeholder loaded instead.\n";
				pMesh = dynamic_pointer_cast<MeshOBJ>(m_GUIDToResourceMap[ConvertGUIDToPair(m_FileNameToGUIDMap["Cube"])]);
			}
			meshes.push_back(std::move(pMesh));
		}
	}
	return meshes;
}

void ResourceManager::tAddJob(std::string filename, std::shared_ptr<Texture2D>* memory, std::vector<std::shared_ptr<MeshOBJ>>* memoryVec)
{
	std::string extension = filename.substr(filename.find_last_of('.'), filename.size() - 1);
	if (extension == ".obj")
	{
		//Set placeholder model in memory parameter.
		std::string cubeName = "Cube.obj";
		*memoryVec = MeshOBJ::Create(cubeName);
	}
	else if (extension == ".png" || extension == ".jpg")
	{
		//Set placeholder texture in memory parameter.
		*memory = Texture2D::Create("Grey.png");
	}
	
	{
		std::unique_lock<std::mutex> lock(m_tQueueMutex);
		m_tQueue.push_back(DBG_NEW JobHolder(tFindResource, filename, extension, memory, memoryVec));
	}
	//Notify a waiting thread.
	m_tCondition.notify_one();
	
}

void ResourceManager::tFindResource(std::string filename, std::string extension, std::shared_ptr<Texture2D>* memory, std::vector<std::shared_ptr<MeshOBJ>>* memoryVec)
{
	if (extension == ".obj")
	{
		std::vector<std::shared_ptr<MeshOBJ>> temp = MeshOBJ::Create(filename);
		
		std::lock_guard<std::mutex> lock(m_FilenameToMutexMap[filename]);
		(*memoryVec).clear();
		for (size_t i = 0; i < temp.size(); i++)
		{
			(*memoryVec).push_back(temp[i]);
		}
		
		//*memoryVec = (MeshOBJ::Create(filename)));
	}
	else if (extension == ".png" || extension == ".jpg")
	{
		//std::lock_guard<std::mutex> lock(m_GUIDToMutexMap[filename]);
		*memory = Texture2D::Create(filename);
	}
}

void ResourceManager::tWaitForJob(
	//Arguments that doesnt change go here. (Or pointers to shared memory)
)
{
	ResourceManager* myself = ResourceManager::Get();
	JobHolder* Job;
	while (!m_tTerminate)
	{
		{
			std::unique_lock<std::mutex> lock(m_tQueueMutex);

			m_tCondition.wait(lock, [myself]()
				{
					return !m_tQueue.empty() || m_tTerminate;
				});
			if (m_tTerminate)
			{
				break;
			}

			Job = m_tQueue.front();
			m_tQueue.pop_front();
		}
		Job->m_job(Job->m_filename, Job->m_extension, Job->m_memory, Job->m_memoryVec);
		delete Job;
	}
}

void ResourceManager::tShutdown()
{
	{
		std::unique_lock<std::mutex> lock(m_tQueueMutex);
		m_tTerminate = true;
	}

	m_tCondition.notify_all();

	for (std::thread& th : m_tWorkers)
	{
		th.join();
	}

	m_tWorkers.clear();

	//stopped = true;
}