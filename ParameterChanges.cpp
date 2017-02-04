#include "ParameterChanges.h"

namespace VSTHost {
ParameterValueQueue::ParameterValueQueue(Steinberg::Vst::ParamID pid) : id(pid), index(-1) {

}

ParameterValueQueue::~ParameterValueQueue() {

}

Steinberg::int32 ParameterValueQueue::GetIndex() const {
	return index;
}

void ParameterValueQueue::SetIndex(Steinberg::int32 i) {
	index = i;
}

void ParameterValueQueue::Clear() {
	values.clear();
	index = -1;

}

Steinberg::Vst::ParamID PLUGIN_API ParameterValueQueue::getParameterId() { 
	return id;
}

Steinberg::int32 PLUGIN_API ParameterValueQueue::getPointCount() {
	return values.size();
}

Steinberg::tresult PLUGIN_API ParameterValueQueue::getPoint(Steinberg::int32 index, Steinberg::int32& sampleOffset /*out*/, Steinberg::Vst::ParamValue& value /*out*/) {
	if (index > 0 && index < static_cast<Steinberg::int32>(values.size())) {
		sampleOffset = values[index].offset;
		value = values[index].value;
		return Steinberg::kResultTrue;
	}
	else
		return Steinberg::kResultFalse;
}

Steinberg::tresult PLUGIN_API ParameterValueQueue::addPoint(Steinberg::int32 sampleOffset, Steinberg::Vst::ParamValue value, Steinberg::int32& index /*out*/) {
	auto size = static_cast<Steinberg::int32>(values.size()), dest_index = size;
	for (Steinberg::int32 i = 0; i < size; ++i) {
		if (values[i].offset == sampleOffset) {
			values[i].value = value;
			index = i;
			return Steinberg::kResultOk;
		}
		else if (values[i].offset > sampleOffset){
			dest_index = i;
			break;
		}
	}
	if (dest_index != size)
		values.emplace_back(value, sampleOffset);
	else
		values.emplace(values.begin() + dest_index, value, sampleOffset);
	index = dest_index;
	return Steinberg::kResultTrue;
}

Steinberg::tresult PLUGIN_API ParameterValueQueue::queryInterface(const Steinberg::TUID _iid, void** obj) {
	QUERY_INTERFACE(_iid, obj, Steinberg::FUnknown::iid, Steinberg::Vst::IParamValueQueue)
	QUERY_INTERFACE(_iid, obj, Steinberg::Vst::IParamValueQueue::iid, Steinberg::Vst::IParamValueQueue)
	*obj = 0;
	return Steinberg::kNoInterface;
}

Steinberg::uint32 PLUGIN_API ParameterValueQueue::addRef() {
	return 1;
}

Steinberg::uint32 PLUGIN_API ParameterValueQueue::release() {
	return 1;
}

ParameterValueQueue::Point::Point(Steinberg::Vst::ParamValue v, Steinberg::int32 o) : value(v), offset(o) {

}

ParameterChanges::ParameterChanges(Steinberg::Vst::IEditController* ec) : param_count(ec->getParameterCount()), used_size(0) {
	Steinberg::Vst::ParameterInfo pi;
	for (Steinberg::int32 i = 0; i < param_count; ++i) {
		ec->getParameterInfo(i, pi);
		queues.emplace(pi.id, ParameterValueQueue(pi.id));
	}
	used.reserve(param_count);
}

ParameterChanges::~ParameterChanges() {

}

ParameterValueQueue* ParameterChanges::GetQueue(Steinberg::Vst::ParamID id) {
	return &(*queues.find(id)).second;
}

void ParameterChanges::ClearQueue() {
	for (Steinberg::int32 i = 0; i < getParameterCount(); ++i)
		used[i]->Clear();
	used.clear();
	used_size = 0;
}

Steinberg::int32 PLUGIN_API ParameterChanges::getParameterCount() {
	return used_size;
}

Steinberg::Vst::IParamValueQueue* PLUGIN_API ParameterChanges::getParameterData(Steinberg::int32 index) {
	if (index >= 0 && index < used_size)
		return used[index];
	else
		return nullptr;
}

Steinberg::Vst::IParamValueQueue* PLUGIN_API ParameterChanges::addParameterData(const Steinberg::Vst::ParamID& pid, Steinberg::int32& index /*out*/) {
	ParameterValueQueue* queue = GetQueue(pid);
	index = queue->GetIndex();
	if (index >= used_size)
		return nullptr;
	else if (index > -1) {
		return used[index];
	}
	else { // index == -1
		used.push_back(queue);
		index = used_size++;
		queue->SetIndex(index);
		return used[index];
	}
}

Steinberg::tresult PLUGIN_API ParameterChanges::queryInterface(const Steinberg::TUID _iid, void** obj) {
	QUERY_INTERFACE(_iid, obj, Steinberg::FUnknown::iid, Steinberg::Vst::IParameterChanges)
	QUERY_INTERFACE(_iid, obj, Steinberg::Vst::IParameterChanges::iid, Steinberg::Vst::IParameterChanges)
	*obj = 0;
	return Steinberg::kNoInterface;
}

Steinberg::uint32 PLUGIN_API ParameterChanges::addRef() {
	return 1;
}

Steinberg::uint32 PLUGIN_API ParameterChanges::release() {
	return 1;
}
};