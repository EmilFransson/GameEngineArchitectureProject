#include "pch.h"
#include "PackageTool.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "DirectXTex/DirectXTex.h"
#include "Graphics.h"


#define PKG_COMPRESS_TEX

std::string PackageTool::Package(const char* dirPath)
{
	std::filesystem::directory_entry Folder = std::filesystem::directory_entry(dirPath);
	if (!Folder.is_directory())
	{
		assert(false);
	}

	//Open a new file for writing the package to.
	//Write a pacakge header to the file.
	std::string newPath = "Packages/" + Folder.path().filename().string() + ".pkg";
	std::ofstream packageFile(newPath, std::ios::binary);
	assert(packageFile);
	char buffer[sizeof(PackageHeader)] = { 0 };

	//Write 0's to PackageHeader
	packageFile.write(buffer, sizeof(PackageHeader));
	uint8_t assetCount = 0;
	uint64_t size = 0;
	size += sizeof(PackageHeader);

	objl::Loader loader;
	for (auto const& dir_entry : std::filesystem::directory_iterator{dirPath})
	{
		if (!dir_entry.path().has_extension())
		{
			assert(false);
		}
		
		std::string filetype = dir_entry.path().extension().string();
		
		if (filetype == ".obj")
		{
			loader.LoadFile(dir_entry.path().string(), dir_entry.path().filename().string());
		}
		else if (filetype == ".png" || filetype == ".jpg")
		{
			assetCount += 1;
			auto texData = PackageTexture(dir_entry.path().string());
#ifdef PKG_COMPRESS_TEX
			CompressTexture(texData);
#endif

			ChunkHeader ch = {
				.type = {'T', 'E', 'X', ' '},
				.chunkSize = sizeof(TextureHeader) + static_cast<uint32_t>(texData.dataVec.size()),
				.readableSize = dir_entry.path().filename().string().length()
			};
			HRESULT hr = CoCreateGuid(&ch.guid);
			if (FAILED(hr)) assert(false); //TODO: actually handle the error
				
			TextureHeader th = {
#ifdef PKG_COMPRESS_TEX
				.textureType = {'B', 'C', '7', ' '},
#else
				.textureType = {'N', 'O', 'R', 'M'},
#endif
				.dataSize = static_cast<uint32_t>(texData.dataVec.size()),
				.width = texData.width,
				.height = texData.height,
				.rowPitch = texData.rowPitch
			};

			//Write the chunkheader
			packageFile.write((char*)(&ch), sizeof(ChunkHeader));
			size += sizeof(ChunkHeader);
			//Write the readable
			packageFile.write(dir_entry.path().filename().string().data(), ch.readableSize);
			size += ch.readableSize;
			//Write the textureheader
			packageFile.write((char*)(&th), sizeof(TextureHeader));
			size += sizeof(TextureHeader);
			//Write the data to the file
			packageFile.write((char*)(texData.dataVec.data()), th.dataSize);
			size += th.dataSize;
		}
	}

	for (objl::Material currentMat : loader.LoadedMaterials)
	{
		assetCount += 1;

		SMaterial sMaterial;
		strcpy_s(sMaterial.fileName, 30, currentMat.fileName.c_str());
		strcpy_s(sMaterial.name, 30, currentMat.name.c_str());
		sMaterial.Ka = currentMat.Ka;
		sMaterial.Kd = currentMat.Kd;
		sMaterial.Ks = currentMat.Ks;
		sMaterial.Ni = currentMat.Ni;
		sMaterial.d = currentMat.d;
		sMaterial.Ns = currentMat.Ns;
		sMaterial.illum = currentMat.illum;
		strcpy_s(sMaterial.map_Ka, 30, currentMat.map_Ka.c_str());
		strcpy_s(sMaterial.map_Kd, 30, currentMat.map_Kd.c_str());
		strcpy_s(sMaterial.map_Ks, 30, currentMat.map_Ks.c_str());
		strcpy_s(sMaterial.map_Ns, 30, currentMat.map_Ns.c_str());
		strcpy_s(sMaterial.map_Ka, 30, currentMat.map_Ka.c_str());
		strcpy_s(sMaterial.map_bump, 30, currentMat.map_bump.c_str());
		strcpy_s(sMaterial.map_d, 30, currentMat.map_d.c_str());

		ChunkHeader ch = {
				.type = {'M', 'A', 'T', ' '},
				.chunkSize = sizeof(MaterialHeader) + sizeof(SMaterial), // 4 channels for DirectX RGBA Textures
				.readableSize = currentMat.fileName.length()
		};
		HRESULT hr = CoCreateGuid(&ch.guid);
		if (FAILED(hr)) assert(false); //TODO: actually handle the error

		MaterialHeader mh = {
			.materialName = { 0 },
			.dataSize = sizeof(SMaterial)
		};
		//Copy in the name.
		currentMat.name.copy(mh.materialName, currentMat.name.size());

		//Write the chunkheader
		packageFile.write((char*)(&ch), sizeof(ChunkHeader));
		size += sizeof(ChunkHeader);
		//Write the readable
		packageFile.write((char*)(currentMat.fileName.data()), ch.readableSize); // Change to stream write operator <<
		size += ch.readableSize;
		//Write the materialheader
		packageFile.write((char*)(&mh), sizeof(MaterialHeader));
		size += sizeof(MaterialHeader);
		//Write the data to the file
		packageFile.write((char*)(&sMaterial), mh.dataSize);
		size += mh.dataSize;
	}

	for (objl::Mesh currentMesh : loader.LoadedMeshes)
	{
		assetCount += 1;
		
		ChunkHeader ch = {
				.type = {'M', 'E', 'S', 'H'},
				.chunkSize = sizeof(MeshHeader) + 
							 static_cast<uint32_t>(currentMesh.Vertices.size() * sizeof(objl::Vertex)) + 
							 static_cast<uint32_t>(currentMesh.Indices.size() * sizeof(unsigned)),

				.readableSize = currentMesh.FileName.length()
		};
		HRESULT hr = CoCreateGuid(&ch.guid);
		if (FAILED(hr)) assert(false); //TODO: actually handle the error

		MeshHeader mh = {
				.meshName = { 0 },
				.materialName = { 0 },
				.verticesDataSize = currentMesh.Vertices.size() * sizeof(objl::Vertex),
				.indicesDataSize = currentMesh.Indices.size() * sizeof(unsigned)
		};
		//Copy in the names.
		currentMesh.MeshName.copy(mh.meshName, currentMesh.MeshName.size());
		currentMesh.MeshMaterial.name.copy(mh.materialName, currentMesh.MeshMaterial.name.size());

		//Write the chunkheader
		packageFile.write((char*)(&ch), sizeof(ChunkHeader));
		size += sizeof(ChunkHeader);
		//Write the readable
		packageFile.write((char*)(currentMesh.FileName.data()), ch.readableSize);
		size += ch.readableSize;
		//Write the textureheader
		packageFile.write((char*)(&mh), sizeof(MeshHeader));
		size += sizeof(MeshHeader);
		//Write the vertices data to the file
		packageFile.write((char*)(currentMesh.Vertices.data()), mh.verticesDataSize);
		size += mh.verticesDataSize;
		//Write the indices data to the file
		packageFile.write((char*)(currentMesh.Indices.data()), mh.indicesDataSize);
		size += mh.indicesDataSize;
	}

	//Update Package Header
	packageFile.seekp(0, std::ios_base::beg);
	PackageHeader ph = {
		.id = { 'P', 'K', 'G'},
		.assetCount = assetCount,
		.size = static_cast<uint32_t>(size)
	};
	packageFile.write((char*)(&ph), sizeof(PackageHeader));

	packageFile.close();
	assert(!packageFile.fail());

	//Zip the package file.

	//Return the filepath to the zip.
	return newPath;
}

PackageTool::PackagedTexture PackageTool::PackageTexture(const std::string& texPath)
{
	PackagedTexture tex = {0};
	int width, height, channels;
	auto imageData = stbi_load(texPath.c_str(), &width, &height, &channels, 0);
	
	tex.width = width;
	tex.height = height;
	tex.rowPitch = width * channels;
	PadTexture(tex, imageData, channels);
	stbi_image_free(imageData);
	return tex;
}

void PackageTool::PadTexture(PackagedTexture& tex, const BYTE* imgData, int channels)
{
	if (channels == 2)
		assert(false);
	if (channels == 4)
	{
		//tex.dataVec.resize(tex.width * tex.height * 4u);
		//memcpy(tex.dataVec.data(), imgData, tex.width * tex.height * 4u);
		tex.dataVec = std::vector(imgData, imgData + tex.width * tex.height * 4u + 1);
	}
	else if (channels == 3)
	{
		for (size_t i = 0; i < tex.width * tex.height * channels; i+=3)
		{
			tex.dataVec.emplace_back(imgData[i + 0]);
			tex.dataVec.emplace_back(imgData[i + 1]);
			tex.dataVec.emplace_back(imgData[i + 2]);
			tex.dataVec.emplace_back(static_cast<BYTE>(0xFF)); // SINCE STARTING PROJECT, added the static_cast.
		}
	}
	else if (channels == 1)
	{
		// Single channel images not supported
		assert(false);
	}
	tex.rowPitch = tex.width * 4;
}

void PackageTool::CompressTexture(PackagedTexture& tex)
{
	DirectX::Image dxImage;
	dxImage.pixels = tex.dataVec.data();
	dxImage.width = tex.width;
	dxImage.height = tex.height;
	dxImage.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxImage.rowPitch = static_cast<size_t>(tex.width) * 4;
	dxImage.slicePitch = tex.width * tex.height * 4;
	
	DirectX::ScratchImage scImage;
	HRESULT hr = DirectX::Compress(Graphics::GetDevice().Get(),
							       dxImage,
							       DXGI_FORMAT_BC7_UNORM_SRGB,
							       DirectX::TEX_COMPRESS_BC7_QUICK | DirectX::TEX_COMPRESS_PARALLEL,
							       1.0,
							       scImage);
	if (!SUCCEEDED(hr))
	{
		assert(false); //For now, just assert
	}

	auto meta = scImage.GetMetadata();
	auto pxSize = scImage.GetPixelsSize();
	auto pixels = scImage.GetPixels();

	tex.dataVec.clear();
	tex.dataVec.resize(pxSize);
	memcpy(tex.dataVec.data(), pixels, pxSize);
	tex.rowPitch = 16 * (tex.width / 4);

	scImage.Release();
}
