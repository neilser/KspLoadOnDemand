#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;

#include "ITexture.h"
#include "Pathing.h"
#include "BitmapFormat.h"

ref class FormatDatabase{
public:
	static List<Func<FileInfo^, array<Byte>^, ITextureBase^>^>^ FormatDetectors = gcnew List<Func<FileInfo^, array<Byte>^, ITextureBase^>^>();
	static Dictionary<Type^, Dictionary<Type^, Func<ITextureBase^, ITextureBase^>^>^>^ FromToConverters = gcnew Dictionary<Type^, Dictionary<Type^, Func<ITextureBase^, ITextureBase^>^>^>();

	static ITextureBase^ Recognize(String^ file, array<Byte>^ data){
		FileInfo^ fi = gcnew FileInfo(file);
		for each(Func<FileInfo^, array<Byte>^, ITextureBase^>^ func in FormatDetectors){
			ITextureBase^ texture = func(fi, data);
			if (texture != nullptr){
				return texture;
			}
		}
		return BitmapFormat::LoadUnknownFile(fi, data);
	}
	static IAssignableTexture^ ConvertToAssignable(ITextureBase^ self){
		IAssignableTexture^ assignableTexture = dynamic_cast<IAssignableTexture^>(self);
		if (assignableTexture != nullptr){
			return assignableTexture;
		}
		else{
			// Todo: Use Path(from,current=>current is IAssignableTexture, grid) instead of just converting to bmp...
			return ConvertTo<BitmapFormat^>(self);
		}
	}
	generic<class TTexture> where TTexture: ITextureBase
	static TTexture ConvertTo(ITextureBase^ self){
		Type^ srcType = self->GetType();
		Type^ trgType = TTexture::typeid;
		ITextureBase^ currentTexture = self;

		// Todo: Cach hot paths...?! Might not worth the effort, since it just called few times per tex and mostly to generate thumbs...
		array<Func<ITextureBase^, ITextureBase^>^>^ path = Pathing<Type^, Func<ITextureBase^, ITextureBase^>^>::FindLinks(srcType, trgType, FromToConverters);
		if (path == nullptr)
		{
			throw gcnew Exception("Couldn't find a way to convert " + srcType->ToString() + " to " + trgType->ToString() + ". Have you added the necessary converters?");
		}
		else
		{
			for each(Func<ITextureBase^, ITextureBase^>^ currentStep in path)
			{
				currentTexture = currentStep(currentTexture);
			}
			return (TTexture)currentTexture;
		}
	}
private:
	ref class GetConverterJobScope{
	public:
		ITextureBase^ texture;
		GetConverterJobScope(ITextureBase^ tex){
			texture = tex;
		}
		AssignableData^ RunConvertion(AssignableFormat^ expectedFormat){
			auto modifiedTexture = texture->ConvertTo<BitmapFormat^>()->Resize(expectedFormat->Width, expectedFormat->Height)->ConvertTo(expectedFormat->Format);
			Func<AssignableFormat^, AssignableData^>^ func = nullptr;
			auto assignable = modifiedTexture->GetAssignableData(nullptr /* should already by in the expected format! */, func);
			if (assignable == nullptr){
				throw gcnew NotSupportedException("Must return assignable!");
			}
			return assignable;
		}
	};
public:
	static Func<AssignableFormat ^, AssignableData ^> ^ GetConverterJob(ITextureBase^ texture){
		return gcnew Func<AssignableFormat^, AssignableData^>(gcnew GetConverterJobScope(texture), &GetConverterJobScope::RunConvertion);
	}
private:
	generic<class TFromTexture, class TToTexture> where TFromTexture : ITextureBase where TToTexture : ITextureBase
	ref class AddConversionScope{
	private:
		Func<TFromTexture, TToTexture>^ Converter;
	public:
		AddConversionScope(Func<TFromTexture, TToTexture>^ converter){
			Converter = converter;
		}
		ITextureBase^ WrappedConverter(ITextureBase^ from){
			return Converter((TFromTexture)from);
		}
	};
public:
	generic<class TFromTexture, class TToTexture> where TFromTexture: ITextureBase where TToTexture: ITextureBase
	static void AddConversion(Func<TFromTexture, TToTexture>^ converter)
	{
		Dictionary<Type^, Func<ITextureBase^, ITextureBase^>^>^ subs;
		if (!FromToConverters->TryGetValue(TFromTexture::typeid, subs))
		{
			subs = gcnew Dictionary<Type^, Func<ITextureBase^, ITextureBase^>^>();
			FromToConverters[TFromTexture::typeid] = subs;
		}
		subs[TToTexture::typeid] = gcnew Func<ITextureBase^, ITextureBase^>(gcnew AddConversionScope<TFromTexture, TToTexture>(converter), &AddConversionScope<TFromTexture, TToTexture>::WrappedConverter);
	}
private:
	generic<class TTexture> where TTexture : ITextureBase
	ref class AddRecognitionScope{
		Func<System::IO::FileInfo^, array<Byte>^, TTexture>^ Recognizer;
	public:
		AddRecognitionScope(Func<System::IO::FileInfo^, array<Byte>^, TTexture>^ recognizer){
			Recognizer = recognizer;
		}
		ITextureBase^ Recognize(System::IO::FileInfo^ file, array<Byte>^ bytes){
			return Recognizer(file, bytes);
		}
	};

 public:
	generic<class TTexture> where TTexture : ITextureBase
	static void AddRecognition(Func<FileInfo^, array<Byte>^, TTexture>^ recognizer)
	{
		if ((TTexture::typeid)->IsAbstract || (TTexture::typeid)->IsInterface)
		{
			throw gcnew ArgumentException("Type has to be the actually used class. Nothing abstract or even interfaces!");
		}
		FormatDetectors->Add(gcnew Func<System::IO::FileInfo^, array<Byte>^, ITextureBase^>(gcnew AddRecognitionScope<TTexture>(recognizer), &AddRecognitionScope<TTexture>::Recognize));
	}

private:
	static BitmapFormat^ RecognizeTGA(FileInfo^ file, array<Byte>^ data);
	static FormatDatabase();
};