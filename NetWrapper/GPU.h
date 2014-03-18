#pragma once


using namespace System;
using namespace System::Runtime::InteropServices;

#include "KSPThread.h"
#include "Work.h"
#include "ITexture.h"
#include "TextureManager.h"

// helper class for GPU related stuff
ref class GPU{
public:
	static void CopyBuffer(unsigned char* src_buffer, int src_pitch, unsigned char* trg_buffer, int trg_pitch, int line_size_in_bytes, int height){
		if (src_pitch == trg_pitch && src_pitch == line_size_in_bytes){
			memcpy(trg_buffer, src_buffer, line_size_in_bytes*height);
		}
		else{
			for (int y = 0; y < height; y++){
				memcpy(trg_buffer, src_buffer, line_size_in_bytes);
				trg_buffer += trg_pitch;
				src_buffer += src_pitch;
			}
		}
	}
	// Unity textures seem to be upside down... we should be able to handle it easily, though...
	// https://developer.oculusvr.com/forums/viewtopic.php?f=37&t=896
	static void CopyBufferInverted(unsigned char* src_buffer, int src_pitch, unsigned char* trg_buffer, int trg_pitch, int line_size_in_bytes, int height){
		CopyBuffer(src_buffer + ((height-1)*src_pitch), -src_pitch, trg_buffer, trg_pitch, line_size_in_bytes, height);
	}
	static String^ Debug_FristBytesToText(unsigned char* data, int bytes){
		auto sb = gcnew System::Text::StringBuilder();
		bool isFirst = true;
		for (int x = 0; x < bytes; x++){
			if (isFirst){
				isFirst = false;
			}
			else{
				sb->Append(",");
			}
			sb->Append(*data);
			data++;
		}
		return sb->ToString();
	}
	static void CopyRawBytesTo(AssignableTarget^ trg, unsigned char* src_buffer, int src_pitch, int line_size_in_bytes, int height, bool srcIsUpsideDown, TextureDebugInfo^ info){
		/*
		auto sb = gcnew System::Text::StringBuilder();
		unsigned char* srcPtr = src_buffer;
		for (int x = src_pitch * height; x > 0; x--){
			sb->AppendLine((*srcPtr).ToString());
			srcPtr++;
		}
		System::IO::File::WriteAllText(info->File + "_DUMP1.txt", sb->ToString());

		sb = gcnew System::Text::StringBuilder();
		if (trg->Inverted != srcIsUpsideDown){
			unsigned char* linePtr = trg->Bits + ((height - 1)*trg->Pitch);
			for (int y = 0; y < height; y++){
				unsigned char* pixelPtr = linePtr;
				for (int x = 0; x < trg->Pitch; x++){
					sb->AppendLine((*pixelPtr).ToString());
					pixelPtr++;
				}
				linePtr = linePtr - trg->Pitch;
			}
		}
		else{
			unsigned char* trgPtr = trg->Bits;
			for (int x = trg->Pitch * height; x > 0; x--){
				sb->AppendLine((*trgPtr).ToString());
				trgPtr++;
			}
		}
		System::IO::File::WriteAllText(info->File + "_DUMP2.txt", sb->ToString());*/
		/*
		for (int x = 0; x < 50; x++){
			sb->AppendLine();
		}*/

		//Logger::LogText(sb->ToString());
		int line_size = trg->Format->BytesPerPixel * trg->Format->Width;
		if (
			(height>trg->Format->Height)
			|| (line_size < src_pitch)
			|| (line_size < trg->Pitch)
			){
			throw gcnew ArgumentException();
		}

		/*

		//auto file = "C:\\ksp_rtLodNew\\" + "LOD_DEBUG_" + ->Replace(":", "")->Replace(" ", "") + ".png";
		
		Logger::LogText(
			Environment::NewLine
			+ Environment::NewLine + "  Copying to GPU: " + info->File + info->Modifiers
			+ Environment::NewLine + "  Saving img copy to " + info->LogFileBaseName);
		auto bmp = gcnew Drawing::Bitmap(trg->Format->Width, trg->Format->Height, AssignableFormat::PixelFormatFromD3DFormat(trg->Format->Format));
		auto bmpData = bmp->LockBits(Drawing::Rectangle(0, 0, bmp->Width, bmp->Height), ImageLockMode::WriteOnly, bmp->PixelFormat);
		auto sourceBytes = Debug_FristBytesToText(src_buffer, 50);
		CopyBuffer(src_buffer, src_pitch, (unsigned char*)bmpData->Scan0.ToPointer(), bmpData->Stride, line_size, bmp->Height);
		bmp->UnlockBits(bmpData);
		bmp->Save(info->LogFileBaseName + "_OUT.png", ImageFormat::Png);
		System::IO::File::Copy(info->File, info->LogFileBaseName + "_SOURCE" + ((gcnew System::IO::FileInfo(info->File))->Extension));
		auto trgBmp = gcnew Drawing::Bitmap(trg->Format->Width, trg->Format->Height, AssignableFormat::PixelFormatFromD3DFormat(trg->Format->Format));
		auto trgData = trgBmp->LockBits(Drawing::Rectangle(0, 0, bmp->Width, bmp->Height), ImageLockMode::WriteOnly, bmp->PixelFormat);
		auto targetBytes = Debug_FristBytesToText((unsigned char*)trgData->Scan0.ToPointer(), 50);
		CopyBuffer(trg->Bits, trg->Pitch, (unsigned char*)trgData->Scan0.ToPointer(), trgData->Stride, line_size, bmp->Height);
		trgBmp->UnlockBits(trgData);
		trgBmp->Save(info->LogFileBaseName + "_TARGET.png", ImageFormat::Png);
		*/
		if (trg->Inverted != srcIsUpsideDown){
			CopyBufferInverted(src_buffer, src_pitch, trg->Bits, trg->Pitch, line_size, height);
		}
		else{
			CopyBuffer(src_buffer, src_pitch, trg->Bits, trg->Pitch, line_size, height);
		}
		/*
		auto writtenBmp = gcnew Drawing::Bitmap(trg->Format->Width, trg->Format->Height, AssignableFormat::PixelFormatFromD3DFormat(trg->Format->Format));
		auto writtenData = writtenBmp->LockBits(Drawing::Rectangle(0, 0, bmp->Width, bmp->Height), ImageLockMode::WriteOnly, bmp->PixelFormat);
		auto writtenBytes = Debug_FristBytesToText((unsigned char*)trg->Bits, 50);
		CopyBuffer(trg->Bits, trg->Pitch, (unsigned char*)writtenData->Scan0.ToPointer(), writtenData->Stride, line_size, bmp->Height);
		writtenBmp->UnlockBits(writtenData);
		writtenBmp->Save(info->LogFileBaseName + "_WRITTE.png", ImageFormat::Png);


		Logger::LogText("Content comparison:"
			+ Environment::NewLine + "SRC: " + sourceBytes
			+ Environment::NewLine + "TRG: " + targetBytes
			+ Environment::NewLine + "WRT: " + writtenBytes
			);
		*/
	}


private:
	static void CopyToTexture(IDirect3DTexture9* texture, AssignableData^ assignableData, AssignableFormat^ format){
		D3DLOCKED_RECT lockedRect;
		if (texture->LockRect(0, &lockedRect, NULL, 0) != D3D_OK){
			throw gcnew NotImplementedException("Todo: Implement error handling (LockRect failed).");
		}
		try{
			assignableData->AssignTo(gcnew AssignableTarget(format, &lockedRect, true /*Unity textures seem to be upside down...*/));
		}
		finally{
			if (texture->UnlockRect(0) != D3D_OK){
				throw gcnew NotImplementedException("Todo: Implement error handling (UnlockRect failed).");
			}
		}
	}
	ref class CreateHighResTextureAsyncScope{
	private:
		Func<AssignableFormat^, AssignableData^>^ delayedCallbackToRun;
		AssignableFormat^ acceptedFormat;
		AssignableData^ delayedAssignableData;
	public:
		IAssignableTexture^ assignableTexture;
		int textureId;
		CreateHighResTextureAsyncScope(IAssignableTexture^ assignable_texture, int texture_id){
			assignableTexture = assignable_texture;
			textureId = texture_id;
		}
		void RunWithAssignable(IDirect3DDevice9* device, AssignableFormat^ format, AssignableData^ data){
			IDirect3DTexture9* dxTexture;
			if (device->CreateTexture(format->Width, format->Height, 0, D3DUSAGE_AUTOGENMIPMAP, format->Format, D3DPOOL::D3DPOOL_MANAGED, &dxTexture, NULL) != D3D_OK){
				throw gcnew NotImplementedException("Todo: Implement error handling (CreateTexture failed).");
			}
			try{
				CopyToTexture(dxTexture, data, format);
			}
			catch (Exception^){
				dxTexture->Release();
				throw;
			}
			TextureManager::OnTextureLoadedToGPU(dxTexture, textureId);
		}
		void RunDelayedSuccessful(IntPtr devicePtr){
			RunWithAssignable((IDirect3DDevice9*)devicePtr.ToPointer(), acceptedFormat, delayedAssignableData);
		}
		void RunDelayedCallback(){

			auto ad = delayedCallbackToRun(acceptedFormat);
			if (ad == nullptr){
				throw gcnew InvalidOperationException("Texture did not return assignable data.");
			}
			else{
				KSPThread::EnqueueJob(gcnew Action<IntPtr>(this, &CreateHighResTextureAsyncScope::RunDelayedSuccessful));
			}
			
		}
		void Run(IntPtr devicePtr){
			auto device = (IDirect3DDevice9*)devicePtr.ToPointer();
			auto format = assignableTexture->GetAssignableFormat();
			UINT width = format->Width, height = format->Height;
			D3DFORMAT texFormat = format->Format;
			if (D3DXCheckTextureRequirements(device, &width, &height, 0, 0, &texFormat, D3DPOOL::D3DPOOL_MANAGED) != D3D_OK){
				throw gcnew NotImplementedException("Todo: Implement error handling (D3DXCheckTextureRequirements failed).");
			}
			auto changedFormat = gcnew AssignableFormat(width, height, texFormat);
			format = changedFormat->SameAs(format) ? nullptr : changedFormat;
			Func<AssignableFormat^, AssignableData^>^ delayedCallback;
			auto assignableData = assignableTexture->GetAssignableData(format, delayedCallback);
			if (assignableData == nullptr){
				if (delayedCallback == nullptr){
					throw gcnew InvalidOperationException("Texture did not return assignable data.");
				}
				else{
					delayedCallbackToRun = delayedCallback;
					acceptedFormat = changedFormat;
					Work::SchedulePriority(gcnew Action(this, &CreateHighResTextureAsyncScope::RunDelayedCallback));
				}
			}
			else{
				RunWithAssignable(device, changedFormat, assignableData);
			}
		}
	};
public:
	static void CreateHighResTextureAsync(IAssignableTexture^ self, int texture_id){
		KSPThread::EnqueueJob(gcnew Action<IntPtr>(gcnew CreateHighResTextureAsyncScope(self, texture_id), &CreateHighResTextureAsyncScope::Run));
	}
private:
	ref class AssignDataToThumbnailAsyncScope{
	public:
		IAssignableTexture^ assignableTexture;
		IDirect3DTexture9* targetTexture;
		AssignDataToThumbnailAsyncScope(IAssignableTexture^ assignable, IDirect3DTexture9* target){
			assignableTexture = assignable;
			targetTexture = target;
		}
		void Run(IntPtr devicePtr){
			D3DSURFACE_DESC texDesc;
			targetTexture->GetLevelDesc(0, &texDesc);
			auto format = gcnew AssignableFormat(texDesc.Width, texDesc.Height, texDesc.Format);
			Func<AssignableFormat^, AssignableData^>^ delayedCallback;
			auto assignableData = assignableTexture->GetAssignableData(
				format->SameAs(assignableTexture->GetAssignableFormat()) ? nullptr : format,
				delayedCallback);
			if (assignableData == nullptr){
				if (delayedCallback == nullptr){
					throw gcnew InvalidOperationException("Texture did not return assignable data.");
				}
				else{
					throw gcnew NotImplementedException("Not sure whether i want support that for AssignToTexture");
				}
			}
			else{
				CopyToTexture(targetTexture, assignableData, format);
				Logger::LogText("success reporting?");
			}

		}
	};
public:
	static void AssignDataToThumbnailAsync(IAssignableTexture^ self, IDirect3DTexture9* texture){
		KSPThread::EnqueueJob(gcnew Action<IntPtr>(gcnew AssignDataToThumbnailAsyncScope(self, texture), &AssignDataToThumbnailAsyncScope::Run));
	}
};