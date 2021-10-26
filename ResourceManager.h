#pragma once
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"
#include "PoolAllocator.h"
#include "BuddyAllocator.hpp"

class ResourceManager
{
public:
#pragma region Deleted ctors
	ResourceManager(const ResourceManager& other) = delete;
	const ResourceManager& operator=(const ResourceManager& other) = delete;
	ResourceManager(const ResourceManager&& other) = delete;
	const ResourceManager& operator=(const ResourceManager&& other) = delete;
#pragma endregion
	static ResourceManager* Get() { return s_Instance; }
	template<typename ResourceType>
	std::shared_ptr<ResourceType> Load(const std::pair<uint64_t, uint64_t>& guid) noexcept;
	template<>
	std::shared_ptr<Texture2D> Load(const std::pair<uint64_t, uint64_t>& guid) noexcept;
	template<>
	std::shared_ptr<MeshOBJ> Load(const std::pair<uint64_t, uint64_t>& guid) noexcept;
	template<>
	std::shared_ptr<Material> Load(const std::pair<uint64_t, uint64_t>& guid) noexcept;
	[[nodiscard]] const bool LoadResourceFromPackage(const std::pair<uint64_t, uint64_t>& guid) noexcept;
	void MapPackageContent() noexcept;
	template<typename ResourceType>
	std::vector<std::shared_ptr<ResourceType>> LoadMultiple(std::string& fileName) noexcept {}
	template<>
	std::vector<std::shared_ptr<MeshOBJ>> LoadMultiple(std::string& objName) noexcept;
	std::pair<uint64_t, uint64_t> ConvertGUIDToPair(const GUID& guid) noexcept;
	GUID ConvertPairToGUID(const std::pair<uint64_t, uint64_t> pGuid) noexcept;
	std::map<std::string, GUID> m_FileNameToGUIDMap;

	//Used for thread initialization and cleanup.
	static void Init();
	static void CleanUp();
private:
	ResourceManager() noexcept
	{
		s_Instance = this;
	}
	~ResourceManager() noexcept = default;
	void FreeMemory() noexcept;
private:
	static ResourceManager* s_Instance;

	std::map<std::pair<uint64_t, uint64_t>, std::shared_ptr<Resource>> m_GUIDToResourceMap;
	std::map<std::pair<uint64_t, uint64_t>, std::string> m_GUIDToPackageMap;
	std::map<std::string, std::vector<std::string>> m_OBJToMeshesMap;
public:
	static std::unique_ptr<PoolAllocator<Texture2D>> m_pTextureAllocator;
	static std::unique_ptr<PoolAllocator<MeshOBJ>> m_pMeshOBJAllocator;
	static std::unique_ptr<PoolAllocator<Material>> m_pMaterialAllocator;
	static void DisplayStateUI();
	static BuddyAllocator buddyAllocator;
	struct BuddyFree { template<class T>void operator()(T* ptr) { buddyAllocator.free(ptr); } };
private:

	struct JobHolder
	{
		JobHolder() = default;
		JobHolder(std::function<void(std::string, std::string, std::shared_ptr<Texture2D>*, std::vector<std::shared_ptr<MeshOBJ>>*)> newJob, std::string filename, std::string extension, std::shared_ptr<Texture2D>* memory, std::vector<std::shared_ptr<MeshOBJ>>* memoryVec)
		{
			m_job = newJob;
			m_filename = filename;
			m_extension = extension;
			m_memory = memory;
			m_memoryVec = memoryVec;
		}
		std::function<void(std::string, std::string, std::shared_ptr<Texture2D>*, std::vector<std::shared_ptr<MeshOBJ>>*)> m_job;
		std::string m_filename;
		std::string m_extension;
		std::shared_ptr<Texture2D>* m_memory;
		std::vector<std::shared_ptr<MeshOBJ>>* m_memoryVec;
	};
	static std::vector<std::thread> m_tWorkers;
	static size_t m_numThreads;

	static std::deque<JobHolder*> m_tQueue;

	static std::mutex m_tQueueMutex;
	static std::condition_variable m_tCondition;
	static bool m_tTerminate;

	static void tWaitForJob();		//The threadfunction.
	static void tFindResource(std::string, std::string, std::shared_ptr<Texture2D>*, std::vector<std::shared_ptr<MeshOBJ>>*);	//Function that is sent into the addJob function. The actual "work" for the thread.
	static void tShutdown();		//Called at shutdown.
		
	static BuddyFree buddyFree;
public:
	std::map<std::string, std::mutex> m_FilenameToMutexMap; //Used for objs
	std::map<std::pair<uint64_t, uint64_t>, std::mutex> m_GUIDToMutexMap;	//Used for textures

	void tAddJob(std::string, std::shared_ptr<Texture2D>*, std::vector<std::shared_ptr<MeshOBJ>>*);	//Called to add a new job to the queue for the threads to do.
};

template<typename ResourceType>
std::shared_ptr<ResourceType> ResourceManager::Load(const std::pair<uint64_t, uint64_t>& guid) noexcept
{
	// Base for template specialization
}
