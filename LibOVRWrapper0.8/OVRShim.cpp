#include "stdafx.h"

#undef OVR_PUBLIC_FUNCTION
#undef OVR_PUBLIC_CLASS
#undef OVR_PRIVATE_FUNCTION
#undef OVR_PRIVATE_CLASS

#if !defined(OVR_DLL_BUILD)
	#define OVR_DLL_BUILD
#endif

#include "../LibOVR0.8/Include/OVR_CAPI_0_8_0.h"
#include "../LibOVR0.8/Include/OVR_CAPI_D3D.h"

#include "shimhelper.h"
#include "OVRShim.h"

OVR_PUBLIC_FUNCTION(ovrResult) ovr_Initialize(const ovrInitParams* params) {
	initChains();

	return rev_Initialize((revInitParams*)params);
}

OVR_PUBLIC_FUNCTION(void) ovr_Shutdown() {
	rev_Shutdown();
}

OVR_PUBLIC_FUNCTION(void) ovr_GetLastErrorInfo(ovrErrorInfo* errorInfo) {
	rev_GetLastErrorInfo((revErrorInfo*)errorInfo);
}

OVR_PUBLIC_FUNCTION(const char*) ovr_GetVersionString() {
	return "OculusSDK0.8";
}

OVR_PUBLIC_FUNCTION(int) ovr_TraceMessage(int level, const char* message) {
	return rev_TraceMessage(level, message);
}

OVR_PUBLIC_FUNCTION(ovrHmdDesc) ovr_GetHmdDesc(ovrSession session) {
	revHmdDesc desc = rev_GetHmdDesc((revSession)session);

	ovrHmdDesc d;

	d.AvailableHmdCaps = desc.AvailableHmdCaps;
	d.AvailableTrackingCaps = desc.AvailableTrackingCaps;
	
	revTrackerDesc tracker = rev_GetTrackerDesc((revSession)session, 0);

	d.CameraFrustumFarZInMeters = tracker.FrustumFarZInMeters;
	d.CameraFrustumHFovInRadians = tracker.FrustumHFovInRadians;
	d.CameraFrustumNearZInMeters = tracker.FrustumNearZInMeters;
	d.CameraFrustumVFovInRadians = tracker.FrustumVFovInRadians;
	
	memcpy(d.DefaultEyeFov, desc.DefaultEyeFov, sizeof(d.DefaultEyeFov));
	d.DefaultHmdCaps = desc.DefaultHmdCaps;
	d.DefaultTrackingCaps = desc.DefaultTrackingCaps;
	d.DisplayRefreshRate = desc.DisplayRefreshRate;
	d.FirmwareMajor = desc.FirmwareMajor;
	d.FirmwareMinor = desc.FirmwareMinor;
	strncpy_s(d.Manufacturer, sizeof(d.Manufacturer), desc.Manufacturer, sizeof(d.Manufacturer) / sizeof(d.Manufacturer[0]));
	memcpy(d.MaxEyeFov, desc.MaxEyeFov, sizeof(d.MaxEyeFov));
	
	d.ProductId = desc.ProductId;
	strncpy_s(d.ProductName, sizeof(d.ProductName), desc.ProductName, sizeof(d.ProductName) / sizeof(d.ProductName[0]));
	d.Resolution = *(ovrSizei *)&desc.Resolution;
	
	strncpy_s(d.SerialNumber, sizeof(d.SerialNumber), desc.SerialNumber, sizeof(d.SerialNumber) / sizeof(d.SerialNumber[0]));
	d.VendorId = desc.VendorId;

	if (desc.Type > 12) {
		d.Type = (ovrHmdType)12;
	}
	else {
		d.Type = (ovrHmdType)desc.Type;
	}

	return d;
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_Create(ovrSession* pSession, ovrGraphicsLuid* pLuid) {
	ovrResult r = rev_Create((revSession*)pSession, (revGraphicsLuid*)pLuid);

	if (!OVR_SUCCESS(r)) {
		return r;
	}

	rev_SetTrackingOriginType(*(revSession*)pSession, revTrackingOrigin_EyeLevel);

	return r;
}

OVR_PUBLIC_FUNCTION(void) ovr_Destroy(ovrSession session) {
	rev_Destroy((revSession)session);
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetSessionStatus(ovrSession session, ovrSessionStatus* sessionStatus) {
	revSessionStatus status;

	ovrResult r = rev_GetSessionStatus((revSession)session, &status);

	sessionStatus->HmdPresent = status.HmdPresent;
	sessionStatus->HasVrFocus = status.IsVisible;

	if (status.ShouldRecenter) {
		rev_RecenterTrackingOrigin((revSession)session);

		//or ovr_ClearShouldRecenterFlag?
	}

	return r;
}

OVR_PUBLIC_FUNCTION(unsigned int) ovr_GetEnabledCaps(ovrSession session) {
	revHmdDesc desc = rev_GetHmdDesc((revSession)session);

	//not possible anymore
	return desc.DefaultHmdCaps;
}

OVR_PUBLIC_FUNCTION(void) ovr_SetEnabledCaps(ovrSession session, unsigned int hmdCaps) {
	//not possible anymore
}

OVR_PUBLIC_FUNCTION(unsigned int) ovr_GetTrackingCaps(ovrSession session) {
	revHmdDesc desc = rev_GetHmdDesc((revSession)session);

	return desc.DefaultTrackingCaps;
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_ConfigureTracking(ovrSession session, unsigned int requestedTrackingCaps,
	unsigned int requiredTrackingCaps) {
	//not used anymore
	return revSuccess;
}

OVR_PUBLIC_FUNCTION(void) ovr_RecenterPose(ovrSession session) {
	rev_RecenterTrackingOrigin((revSession)session);
}

void copyPose(ovrPosef* dest, const revPosef* source) {
	dest->Orientation = *(ovrQuatf*)&source->Orientation;
	dest->Position = *(ovrVector3f*)&source->Position;
}

void copyPoseR(revPosef* dest, const ovrPosef* source) {
	dest->Orientation = *(revQuatf*)&source->Orientation;
	dest->Position = *(revVector3f*)&source->Position;
}

void copyPoseState(ovrPoseStatef* dest, const revPoseStatef* source) {
	dest->AngularAcceleration = *(ovrVector3f*)&source->AngularAcceleration;
	dest->AngularVelocity = *(ovrVector3f*)&source->AngularVelocity;
	dest->LinearAcceleration = *(ovrVector3f*)&source->LinearAcceleration;
	dest->LinearVelocity = *(ovrVector3f*)&source->LinearVelocity;
	copyPose(&(dest->ThePose), &(source->ThePose));
	dest->TimeInSeconds = source->TimeInSeconds;
}

OVR_PUBLIC_FUNCTION(ovrTrackingState) ovr_GetTrackingState(ovrSession session, double absTime, ovrBool latencyMarker) {
	revTrackingState state = rev_GetTrackingState((revSession)session, absTime, latencyMarker);
	revTrackerPose tpose = rev_GetTrackerPose((revSession)session, 0);

	ovrTrackingState r;	
	copyPose(&(r.CameraPose), &(tpose.Pose));
	r.CameraPose.Orientation = *(ovrQuatf*)&tpose.Pose.Orientation;
	r.CameraPose.Position = *(ovrVector3f*)&tpose.Pose.Position;
	copyPoseState(&(r.HandPoses[0]), &(state.HandPoses[0]));
	copyPoseState(&(r.HandPoses[1]), &(state.HandPoses[1]));

	r.HandStatusFlags[0] = state.HandStatusFlags[0];
	r.HandStatusFlags[1] = state.HandStatusFlags[1];

	copyPoseState(&(r.HeadPose), &(state.HeadPose));

	//r.LastCameraFrameCounter not filled

	copyPose(&(r.LeveledCameraPose), &(tpose.LeveledPose));

	//r.RawSensorData not filled
	r.StatusFlags = state.StatusFlags | ovrStatus_CameraPoseTracked | ovrStatus_PositionConnected | ovrStatus_HmdConnected;
	
	return r;
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetInputState(ovrSession session, unsigned int controllerTypeMask, ovrInputState* inputState) {
	revInputState state;

	ovrResult res = rev_GetInputState((revSession)session, (revControllerType)controllerTypeMask, &state);

	if (res < 0) {
		return res;
	}

	inputState->Buttons = state.Buttons; // needs mapping?
	inputState->ConnectedControllerTypes = rev_GetConnectedControllerTypes((revSession)session);
	inputState->HandTrigger[0] = state.HandTrigger[0];
	inputState->HandTrigger[1] = state.HandTrigger[1];

	inputState->IndexTrigger[0] = state.IndexTrigger[0];
	inputState->IndexTrigger[1] = state.IndexTrigger[1];

	inputState->Thumbstick[0] = *(ovrVector2f *)&state.Thumbstick[0];
	inputState->Thumbstick[1] = *(ovrVector2f *)&state.Thumbstick[1];

	inputState->TimeInSeconds = state.TimeInSeconds;
	inputState->Touches = state.Touches;

	return res;
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_SetControllerVibration(ovrSession session, unsigned int controllerTypeMask,
	float frequency, float amplitude) {
	return rev_SetControllerVibration((revSession)session, (revControllerType)controllerTypeMask, frequency, amplitude);
}

OVR_PUBLIC_FUNCTION(void) ovr_DestroySwapTextureSet(ovrSession session, ovrSwapTextureSet* textureSet) {	
	rev_DestroyTextureSwapChain((revSession)session, getChain((revSession)session, textureSet)->swapChain);

	removeChain((revSession)session, textureSet);
}

OVR_PUBLIC_FUNCTION(void) ovr_DestroyMirrorTexture(ovrSession session, ovrTexture* mirrorTexture) {
	revMirrorTexture* mirror = getMirror();

	rev_DestroyMirrorTexture((revSession)session, *mirror);

	setMirror(NULL);

	if (mirrorTexture->Header.API == ovrRenderAPI_D3D11) {
		union ovrD3D11Texture* ovrtext = (union ovrD3D11Texture*)mirrorTexture;

		ovrtext->D3D11.pTexture->Release();
	}
}

OVR_PUBLIC_FUNCTION(ovrSizei) ovr_GetFovTextureSize(ovrSession session, ovrEyeType eye, ovrFovPort fov,
	float pixelsPerDisplayPixel) {
	revFovPort fport;
	fport.DownTan = fov.DownTan;
	fport.LeftTan = fov.LeftTan;
	fport.RightTan = fov.RightTan;
	fport.UpTan = fov.UpTan;

	return *(ovrSizei *)&rev_GetFovTextureSize((revSession)session, (revEyeType)eye, fport, pixelsPerDisplayPixel);
}

OVR_PUBLIC_FUNCTION(ovrEyeRenderDesc) ovr_GetRenderDesc(ovrSession session,
	ovrEyeType eyeType, ovrFovPort fov) {

	revFovPort fport;
	fport.DownTan = fov.DownTan;
	fport.LeftTan = fov.LeftTan;
	fport.RightTan = fov.RightTan;
	fport.UpTan = fov.UpTan;

	revEyeRenderDesc desc = rev_GetRenderDesc((revSession)session, (revEyeType)eyeType, fport);

	ovrEyeRenderDesc r;

	r.DistortedViewport = *(ovrRecti*)&desc.DistortedViewport;
	r.Eye = (ovrEyeType)desc.Eye;
	r.Fov.DownTan = desc.Fov.DownTan;
	r.Fov.LeftTan = desc.Fov.LeftTan;
	r.Fov.RightTan = desc.Fov.RightTan;
	r.Fov.UpTan = desc.Fov.UpTan;
	r.HmdToEyeViewOffset = *(ovrVector3f*)&desc.HmdToEyeOffset;
	r.PixelsPerTanAngleAtCenter = *(ovrVector2f *)&desc.PixelsPerTanAngleAtCenter;

	return r;
}

revTextureSwapChain renderChain(revSession session, ovrSwapTextureSet* ts)
{
	ovrTextureSwapChainWrapper* chainwrapper = getChain(session, ts);

	int currentIndex = 0;
	rev_GetTextureSwapChainCurrentIndex(session, chainwrapper->swapChain, &currentIndex);

	CopyTexture(chainwrapper->pContext, chainwrapper->textures[currentIndex], &ts->Textures[ts->CurrentIndex]);	
	
	rev_CommitTextureSwapChain(session, chainwrapper->swapChain);

	return chainwrapper->swapChain;
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_SubmitFrame(ovrSession session, long long frameIndex,
	const ovrViewScaleDesc* viewScaleDesc,
	ovrLayerHeader const * const * layerPtrList, unsigned int layerCount) {

	//ovrLayerType 2, 6 do not exists anymore. max layer count is 16 instead of 32

	unsigned int trueLayerCount = 0;
	for (unsigned int i = 0;i < layerCount;i++) {
		if (layerPtrList[i] != NULL) {
			trueLayerCount++;
		}
	}

	if (trueLayerCount > 16) {
		trueLayerCount = 16; //ignore layer counts > 16
	}	

	revLayerHeader** newlayers = (revLayerHeader**)malloc(sizeof(revLayerHeader*) * trueLayerCount);
	
	unsigned int np = 0;

	for (unsigned int i = 0;i < layerCount;i++) {
		const ovrLayerHeader* layer = layerPtrList[i];

		if (layer == NULL) {
			continue;
		}

		if (layer->Type == ovrLayerType_EyeFov) {
			const ovrLayerEyeFov* oldelayer = (const ovrLayerEyeFov*)layer;
			revLayerEyeFov *elayer = (revLayerEyeFov*)malloc(sizeof(revLayerEyeFov));

			//if both eyes use same swaptextureset
			if (oldelayer->ColorTexture[0] == oldelayer->ColorTexture[1]) {
				elayer->ColorTexture[0] = renderChain((revSession)session, oldelayer->ColorTexture[0]);
				elayer->ColorTexture[1] = elayer->ColorTexture[0];
			} else {
				elayer->ColorTexture[0] = renderChain((revSession)session, oldelayer->ColorTexture[0]);
				elayer->ColorTexture[1] = renderChain((revSession)session, oldelayer->ColorTexture[1]);
			}		
			
			elayer->Fov[0].DownTan = oldelayer->Fov[0].DownTan;
			elayer->Fov[0].LeftTan = oldelayer->Fov[0].LeftTan;
			elayer->Fov[0].UpTan = oldelayer->Fov[0].UpTan;
			elayer->Fov[0].RightTan = oldelayer->Fov[0].RightTan;
			elayer->Fov[1].DownTan = oldelayer->Fov[1].DownTan;
			elayer->Fov[1].LeftTan = oldelayer->Fov[1].LeftTan;
			elayer->Fov[1].UpTan = oldelayer->Fov[1].UpTan;
			elayer->Fov[1].RightTan = oldelayer->Fov[1].RightTan;

			elayer->Header.Flags = oldelayer->Header.Flags;
			elayer->Header.Type = (revLayerType)oldelayer->Header.Type;

			copyPoseR(&elayer->RenderPose[0], &oldelayer->RenderPose[0]);
			copyPoseR(&elayer->RenderPose[1], &oldelayer->RenderPose[1]);

			elayer->SensorSampleTime = oldelayer->SensorSampleTime;
			elayer->Viewport[0] = *(revRecti*)&oldelayer->Viewport[0];
			elayer->Viewport[1] = *(revRecti*)&oldelayer->Viewport[1];

			newlayers[np] = (revLayerHeader*)elayer;
		}
		else if (layer->Type == ovrLayerType_EyeMatrix) {
			const ovrLayerEyeMatrix* oldelayer = (const ovrLayerEyeMatrix*)layer;
			revLayerEyeMatrix *elayer = (revLayerEyeMatrix*)malloc(sizeof(revLayerEyeMatrix));

			//if both eyes use same swaptextureset
			if (oldelayer->ColorTexture[0] == oldelayer->ColorTexture[1]) {
				elayer->ColorTexture[0] = renderChain((revSession)session, oldelayer->ColorTexture[0]);
				elayer->ColorTexture[1] = elayer->ColorTexture[0];
			}
			else {
				elayer->ColorTexture[0] = renderChain((revSession)session, oldelayer->ColorTexture[0]);
				elayer->ColorTexture[1] = renderChain((revSession)session, oldelayer->ColorTexture[1]);
			}

			elayer->Matrix[0] = *(revMatrix4f *)&oldelayer->Matrix[0];
			elayer->Matrix[1] = *(revMatrix4f *)&oldelayer->Matrix[1];

			elayer->Header.Flags = oldelayer->Header.Flags;
			elayer->Header.Type = (revLayerType)oldelayer->Header.Type;

			copyPoseR(&elayer->RenderPose[0], &oldelayer->RenderPose[0]);
			copyPoseR(&elayer->RenderPose[1], &oldelayer->RenderPose[1]);

			elayer->SensorSampleTime = oldelayer->SensorSampleTime;
			elayer->Viewport[0] = *(revRecti*)&oldelayer->Viewport[0];
			elayer->Viewport[1] = *(revRecti*)&oldelayer->Viewport[1];

			newlayers[np] = (revLayerHeader*)elayer;
		}
		else if (layer->Type == ovrLayerType_Quad) {
			const ovrLayerQuad* oldelayer = (const ovrLayerQuad*)layer;
			revLayerQuad *elayer = (revLayerQuad*)malloc(sizeof(revLayerQuad));

			elayer->Header.Type = revLayerType_Quad;
			elayer->Header.Flags = layer->Flags;

			elayer->ColorTexture = renderChain((revSession)session, oldelayer->ColorTexture);
			
			copyPoseR(&elayer->QuadPoseCenter, &oldelayer->QuadPoseCenter);

			elayer->QuadSize = *(revVector2f *)&oldelayer->QuadSize;

			newlayers[np] = (revLayerHeader*)elayer;
		}
		else if (layer->Type == ovrLayerType_Disabled) {			
			revLayerHeader *elayer = (revLayerHeader*)malloc(sizeof(revLayerHeader));

			elayer->Flags = layer->Flags;
			elayer->Type = (revLayerType)layer->Type;

			newlayers[np] = (revLayerHeader*)elayer;
		}
		else {
			continue; //ignore unsupported layers
		}

		np++;

		if (np > 15) {
			break;
		}
	}
	
	ovrResult r = rev_SubmitFrame((revSession)session, frameIndex, (const revViewScaleDesc*)viewScaleDesc, newlayers, trueLayerCount);

	for (unsigned int i = 0;i < trueLayerCount;i++) {
		free(newlayers[i]);
	}

	free(newlayers);

	return r;
}

OVR_PUBLIC_FUNCTION(double) ovr_GetPredictedDisplayTime(ovrSession session, long long frameIndex) {
	return rev_GetPredictedDisplayTime((revSession)session, frameIndex);
}

OVR_PUBLIC_FUNCTION(double) ovr_GetTimeInSeconds() {
	return rev_GetTimeInSeconds();
}

OVR_PUBLIC_FUNCTION(void) ovr_ResetBackOfHeadTracking(ovrSession session) {
	//nothing
}

OVR_PUBLIC_FUNCTION(void) ovr_ResetMulticameraTracking(ovrSession session) {
	//nothing
}

OVR_PUBLIC_FUNCTION(ovrBool) ovr_GetBool(ovrSession session, const char* propertyName, ovrBool defaultVal) {
	return rev_GetBool((revSession)session, propertyName, defaultVal);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetBool(ovrSession session, const char* propertyName, ovrBool value) {
	return rev_SetBool((revSession)session, propertyName, value);
}

OVR_PUBLIC_FUNCTION(int) ovr_GetInt(ovrSession session, const char* propertyName, int defaultVal) {
	return rev_GetInt((revSession)session, propertyName, defaultVal);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetInt(ovrSession session, const char* propertyName, int value) {
	return rev_SetInt((revSession)session, propertyName, value);
}

OVR_PUBLIC_FUNCTION(float) ovr_GetFloat(ovrSession session, const char* propertyName, float defaultVal) {
	if (strcmp(propertyName, OVR_KEY_IPD) == 0) {
		float values[2];
		rev_GetFloatArray((revSession)session, REV_KEY_NECK_TO_EYE_DISTANCE_, values, 2);

		return values[0] + values[1];
	}

	return rev_GetFloat((revSession)session, propertyName, defaultVal);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetFloat(ovrSession session, const char* propertyName, float value) {
	if (strcmp(propertyName, OVR_KEY_IPD) == 0) {
		return ovrFalse;
	}

	return rev_SetFloat((revSession)session, propertyName, value);
}

OVR_PUBLIC_FUNCTION(unsigned int) ovr_GetFloatArray(ovrSession session, const char* propertyName,
	float values[], unsigned int valuesCapacity) {
	return rev_GetFloatArray((revSession)session, propertyName, values, valuesCapacity);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetFloatArray(ovrSession session, const char* propertyName,
	const float values[], unsigned int valuesSize) {
	return rev_SetFloatArray((revSession)session, propertyName, values, valuesSize);
}

OVR_PUBLIC_FUNCTION(const char*) ovr_GetString(ovrSession session, const char* propertyName,
	const char* defaultVal) {
	return rev_GetString((revSession)session, propertyName, defaultVal);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovr_SetString(ovrSession session, const char* propertyName,
	const char* value) {
	return rev_SetString((revSession)session, propertyName, value);
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_SetQueueAheadFraction(ovrSession session, float queueAheadFraction) {
	return rev_SetQueueAheadFraction((revSession)session, queueAheadFraction);
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_Lookup(const char* name, void** data) {
	return rev_Lookup(name, data);
}

//these two functions below are just for debugging purposes
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetTextureSwapChainCurrentIndex(ovrSession session, ovrSwapTextureSet* textureSet, int* currentIndex) {
	revTextureSwapChain chain = getChain((revSession)session, textureSet)->swapChain;

	return rev_GetTextureSwapChainCurrentIndex((revSession)session, chain, currentIndex);
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_CommitTextureSwapChain(ovrSession session, ovrSwapTextureSet* textureSet) {
	return rev_CommitTextureSwapChain((revSession)session, getChain((revSession)session, textureSet)->swapChain);
}