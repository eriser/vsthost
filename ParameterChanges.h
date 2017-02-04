#ifndef PARAMETERCHANGES_H
#define PARAMETERCHANGES_H

#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <vector>
#include <map>

namespace VSTHost {
class ParameterValueQueue : public Steinberg::Vst::IParamValueQueue {
public:
	ParameterValueQueue(Steinberg::Vst::ParamID pid);
	~ParameterValueQueue();
	Steinberg::int32 GetIndex() const;
	void SetIndex(Steinberg::int32 i);
	void Clear();
	Steinberg::Vst::ParamID PLUGIN_API getParameterId();
	Steinberg::int32 PLUGIN_API getPointCount();
	Steinberg::tresult PLUGIN_API getPoint(Steinberg::int32 index, Steinberg::int32& sampleOffset /*out*/, Steinberg::Vst::ParamValue& value /*out*/);
	Steinberg::tresult PLUGIN_API addPoint(Steinberg::int32 sampleOffset, Steinberg::Vst::ParamValue value, Steinberg::int32& index /*out*/);
	Steinberg::tresult PLUGIN_API queryInterface(const Steinberg::TUID _iid, void** obj);
	Steinberg::uint32 PLUGIN_API addRef();
	Steinberg::uint32 PLUGIN_API release();

	void clear();

private:
	const Steinberg::Vst::ParamID id;
	Steinberg::int32 index;
	struct Point
	{
		Point(Steinberg::Vst::ParamValue v, Steinberg::int32 o);
		Steinberg::Vst::ParamValue value;
		Steinberg::int32 offset;
	};
	std::vector<Point> values;
};

class ParameterChanges : public Steinberg::Vst::IParameterChanges {
public:
	ParameterChanges(Steinberg::Vst::IEditController* ec);
	~ParameterChanges();
	ParameterValueQueue* GetQueue(Steinberg::Vst::ParamID id);
	void ClearQueue();
	Steinberg::int32 PLUGIN_API getParameterCount();
	Steinberg::Vst::IParamValueQueue* PLUGIN_API getParameterData(Steinberg::int32 index);
	Steinberg::Vst::IParamValueQueue* PLUGIN_API addParameterData(const Steinberg::Vst::ParamID& pid, Steinberg::int32& index /*out*/);
	Steinberg::tresult PLUGIN_API queryInterface(const Steinberg::TUID _iid, void** obj);
	Steinberg::uint32 PLUGIN_API addRef();
	Steinberg::uint32 PLUGIN_API release();
private:
	const Steinberg::int32 param_count;
	std::map<Steinberg::Vst::ParamID, ParameterValueQueue> queues;
	Steinberg::int32 used_size;
	std::vector<ParameterValueQueue*> used;
};
} // namespace

#endif