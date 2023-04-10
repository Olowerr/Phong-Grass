#pragma once
#include <chrono>

namespace Okay
{
	class Time
	{
	public:
		Time() = delete;

		static inline float getDT();
		static inline void setTimeScale(float timeScale);
		static inline float getUpTime();

		static inline float getApplicationUpTime();
		static inline float getApplicationDT();

		static inline void measure();
		static inline void start();

		using Duration = std::chrono::duration<float>;
		using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
		static inline TimePoint getTimeNow();

	private:
		static inline TimePoint frameStart;
		static inline Duration dtApp = Duration(0.f);
		static inline Duration upTimeApp = Duration(0.f);
		static inline Duration upTime = Duration(0.f);
		static inline float timeScale = 1.f;
	};

	inline Time::TimePoint Time::getTimeNow() { return std::chrono::system_clock::now(); }
	
	inline float Time::getDT() { return Time::dtApp.count() * Time::timeScale; }
	inline void Time::setTimeScale(float timeScale) { Time::timeScale = timeScale; }

	inline float Time::getApplicationUpTime() { return Time::upTimeApp.count(); }
	inline float Time::getApplicationDT() { return Time::dtApp.count(); }
	inline float Time::getUpTime() { return Time::upTime.count(); }
	
	inline void Time::measure()
	{
		const TimePoint timeNow = getTimeNow();

		Time::dtApp = timeNow - Time::frameStart;
		Time::frameStart = timeNow;

		Time::upTime += Time::dtApp * Time::timeScale;
		Time::upTimeApp += Time::dtApp;
	}

	inline void Time::start()
	{
		Time::dtApp = Duration(0.f);
		Time::frameStart = getTimeNow();
	}
}