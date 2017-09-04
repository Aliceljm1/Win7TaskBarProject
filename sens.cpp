// Main
#include "stdafx.h"

// Sensor and Location functions

ISensorManager* sm = 0;
ISensor* LightSensor = 0;
CLSID LightSensorID;

void SensorDataUpdate(ISensor* s,ISensorDataReport* pData);


// Sensor Manager Events Implementation
class MySensorManagerEvents : public ISensorManagerEvents
	{
	private:
		unsigned long ref;

	public:

		MySensorManagerEvents()
			{
			ref = 0;
			AddRef();
			}

		// IUnknown
		ULONG __stdcall AddRef()
			{
			return ++ref;
			}

		ULONG __stdcall Release()
			{
			return --ref;
			}
		HRESULT __stdcall QueryInterface(const IID& id,void**p)
			{
			if (id == IID_IUnknown || id == __uuidof(ISensorManagerEvents))
				{
				*p = this;
				AddRef();
				return S_OK;
				}
			return E_NOINTERFACE;
			}

		HRESULT __stdcall OnSensorEnter(
			ISensor *pSensor,
			SensorState state)
			{
			UNREFERENCED_PARAMETER(pSensor);
			UNREFERENCED_PARAMETER(state);
			InvalidateRect(MainWindow,0,0);
			UpdateWindow(MainWindow);
			return S_OK;
			}
	};
MySensorManagerEvents sme;

// Sensor Events Implementation
class MySensorEvents : public ISensorEvents
	{
	private:
		unsigned long ref;

	public:

		MySensorEvents()
			{
			ref = 0;
			AddRef();
			}

		// IUnknown
		ULONG __stdcall AddRef()
			{
			return ++ref;
			}

		ULONG __stdcall Release()
			{
			return --ref;
			}
		HRESULT __stdcall QueryInterface(const IID& id,void**p)
			{
			if (id == IID_IUnknown || id == __uuidof(ISensorEvents))
				{
				*p = this;
				AddRef();
				return S_OK;
				}
			return E_NOINTERFACE;
			}

		// Sensor Events
		HRESULT __stdcall OnEvent(ISensor *pSensor,REFGUID eventID,IPortableDeviceValues *pEventData)
			{
			UNREFERENCED_PARAMETER(pEventData);
			UNREFERENCED_PARAMETER(pSensor);
			UNREFERENCED_PARAMETER(eventID);
			return S_OK;
			}
		HRESULT __stdcall OnDataUpdated(ISensor *pSensor,ISensorDataReport *pNewData)
			{
			SensorDataUpdate(pSensor,pNewData);
			return S_OK;
			}
		HRESULT __stdcall OnLeave(REFSENSOR_ID sensorID)
			{
			// Free sensors if released
			if (sensorID == LightSensorID)
				{
				LightSensor->Release();
				LightSensor = 0;
				}
			InvalidateRect(MainWindow,0,0);
			UpdateWindow(MainWindow);
			return S_OK;
			}
		HRESULT __stdcall OnStateChanged(ISensor *pSensor,SensorState state)
			{
			UNREFERENCED_PARAMETER(state);
			SensorDataUpdate(pSensor,0);
			return S_OK;
			}

	};
MySensorEvents ses;

bool SensorsInit(HWND hh)
	{
	UNREFERENCED_PARAMETER(hh);
	HRESULT hr = 0;
	CoCreateInstance(CLSID_SensorManager,0,CLSCTX_ALL,__uuidof(ISensorManager),(void**)&sm);
	if (!sm)
		return false;
	sm->SetEventSink(&sme);


	GUID pguid[2];
	pguid[0] = SENSOR_EVENT_DATA_UPDATED;
//	pguid[0] = SENSOR_EVENT_STATE_CHANGED;

	// Check to find light sensor
	ISensorCollection* ic = 0;
	sm->GetSensorsByCategory(SENSOR_CATEGORY_LIGHT,&ic);
	if (ic)
		{
		ULONG pC = 0;
		hr = ic->GetCount(&pC);
		if (pC)
			{
			// Get the first one
			ic->GetAt(0,&LightSensor);
			if (LightSensor)
				{
				LightSensor->GetID(&LightSensorID);
				hr = LightSensor->SetEventSink(&ses);
				GUID pguid[2];
				hr = LightSensor->SetEventInterest(pguid,1);
				}
			}
		ic->Release();
		}
	ic = 0;


	return true;
	}

void SensorDataUpdate(ISensor* s,ISensorDataReport* pData)
	{
	// Force Redraw
	UNREFERENCED_PARAMETER(s);
	UNREFERENCED_PARAMETER(pData);
	InvalidateRect(MainWindow,0,FALSE);
	UpdateWindow(MainWindow);
	}


float GetLightConditionLux()
	{
	// Find a light sensor
	if (!LightSensor)
		return -1.0f;

	// Get the value
	ISensorDataReport* d = 0;
	LightSensor->GetData(&d);
	if (!d)
		return -1.0f;

	PROPVARIANT pv;
	PropVariantInit(&pv);
	pv.vt = VT_R4;
	d->GetSensorValue(SENSOR_DATA_TYPE_LIGHT_LEVEL_LUX,&pv);
	d->Release();
	return pv.fltVal;
	}

POINTF3 GetLocation()
	{
	HRESULT hr = 0;
	POINTF3 p = {0};

	// Create the ILocation
	ILocation* lm = 0;
	CoCreateInstance(CLSID_Location,0,CLSCTX_ALL,__uuidof(ILocation),(void**)&lm);

	if (!lm)
		return p;

	ILocationReport* lmr = 0;
	IID REPORT_TYPES[] = { IID_ILatLongReport };

	hr = lm->RequestPermissions(MainWindow,REPORT_TYPES,1,TRUE);

	LOCATION_REPORT_STATUS status = REPORT_NOT_SUPPORTED; 
	hr = lm->GetReportStatus(IID_ILatLongReport, &status);

	if (status == REPORT_RUNNING)
		hr = lm->GetReport(__uuidof(ILatLongReport),&lmr);
	if (lmr)
		{
		ILatLongReport* llr = 0;
		hr = lmr->QueryInterface(__uuidof(ILatLongReport),(void**)&llr);
		if (llr)
			{
			DOUBLE xx = 0,yy = 0,zz = 0;

			llr->GetLongitude(&xx);
			llr->GetLatitude(&yy);
			llr->GetAltitude(&zz);

			p.x = xx;
			p.y = yy;
			p.z = zz;

			llr->Release();
			}
		lmr->Release();
		}

	lm->Release();
	lm = 0;
	return p;
	}


void SensorsEnd()
	{
	if (LightSensor)
		LightSensor->Release();
	LightSensor = 0;
	if (sm)
		sm->Release();
	sm = 0;
	}
