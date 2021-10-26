#include <string>	

enum SIDText
	{
		sidNoBackgroundCreated=0,
		sidNoGalleryCreated,
		sidNoGalleryFound,
		sidGeneralDirectDrawError,
		sidGeneralBackgroundDrawingError,
		sidGeneralSpriteDrawingError,
		sidNoImagesDirectory,
		sidEmptyGalleryString,
		sidConvertingGraphics,
		sidDirectDrawNotCreated,
		sidGalleryNotCreated,
		sidGalleryNotFound,
		sidGalleryUnknownType,
		sidMapImageNotCreated,
		sidMissingGallery,
		sidGalleryNotSpecified,
		sidCreatureGalleryCorrupt,
		sidNoSecondaryCameraCreated,
		sidBitmapSizeTooSmall,
		sidInvalidBasePose,
		sidDodgyPixelFormat1,
		sidDodgyPixelFormat2
	};

	const std::string theDisplayErrorTag("display_engine");

