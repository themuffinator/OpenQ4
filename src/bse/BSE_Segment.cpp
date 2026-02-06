// BSE_Segment.cpp
//




#include "BSE_Envelope.h"
#include "BSE_Particle.h"
#include "BSE_SpawnDomains.h"
#include "BSE.h"

#include <math.h>
#include <stdint.h>

#include "../renderer/Model_local.h"
#include "../renderer/tr_local.h"
#include "../framework/Session.h"

namespace {
static const int kMaxVisualParticles = 512;

ID_INLINE uint32_t BSEHash32(uint32_t seed) {
	seed ^= seed >> 16;
	seed *= 0x7feb352dU;
	seed ^= seed >> 15;
	seed *= 0x846ca68bU;
	seed ^= seed >> 16;
	return seed;
}

ID_INLINE float HashUnit(uint32_t seed) {
	return (BSEHash32(seed) & 0x00FFFFFF) * (1.0f / 16777215.0f);
}

ID_INLINE float HashRange(uint32_t seed, float minValue, float maxValue) {
	return minValue + (maxValue - minValue) * HashUnit(seed);
}

ID_INLINE float LerpFloat(float a, float b, float t) {
	return a + (b - a) * t;
}

ID_INLINE void ApplyRelative(const rvParticleParms* parms, float* value, const float* initValue, int count) {
	if (!parms || (parms->mFlags & PPFLAG_RELATIVE) == 0) {
		return;
	}
	for (int i = 0; i < count; ++i) {
		value[i] += initValue[i];
	}
}

ID_INLINE void SampleUnitSphere(uint32_t seed, idVec3& outDir) {
	const float z = HashRange(seed + 1, -1.0f, 1.0f);
	const float phi = HashRange(seed + 2, 0.0f, idMath::TWO_PI);
	const float r = idMath::Sqrt(Max(0.0f, 1.0f - z * z));
	outDir.x = r * idMath::Cos(phi);
	outDir.y = r * idMath::Sin(phi);
	outDir.z = z;
}

ID_INLINE void SampleParticleParms(const rvParticleParms* parms, int parmCount, uint32_t seed, float* outValue) {
	outValue[0] = 0.0f;
	outValue[1] = 0.0f;
	outValue[2] = 0.0f;
	if (!parms || parmCount <= 0) {
		return;
	}

	const idVec3& mins = parms->mMins;
	const idVec3& maxs = parms->mMaxs;
	const int spawnBase = parms->mSpawnType & ~0x3;

	switch (spawnBase) {
	case SPF_NONE_0:
		return;
	case SPF_ONE_0:
		outValue[0] = 1.0f;
		outValue[1] = (parmCount > 1) ? 1.0f : 0.0f;
		outValue[2] = (parmCount > 2) ? 1.0f : 0.0f;
		return;
	case SPF_POINT_0:
		outValue[0] = mins.x;
		if (parmCount > 1) {
			outValue[1] = mins.y;
		}
		if (parmCount > 2) {
			outValue[2] = mins.z;
		}
		return;
	case SPF_LINEAR_0: {
		const float t = HashUnit(seed + 17);
		outValue[0] = LerpFloat(mins.x, maxs.x, t);
		if (parmCount > 1) {
			outValue[1] = LerpFloat(mins.y, maxs.y, t);
		}
		if (parmCount > 2) {
			outValue[2] = LerpFloat(mins.z, maxs.z, t);
		}
		return;
	}
	case SPF_BOX_0:
	case SPF_MODEL_0:
		outValue[0] = HashRange(seed + 11, mins.x, maxs.x);
		if (parmCount > 1) {
			outValue[1] = HashRange(seed + 13, mins.y, maxs.y);
		}
		if (parmCount > 2) {
			outValue[2] = HashRange(seed + 15, mins.z, maxs.z);
		}
		return;
	case SPF_SURFACE_BOX_0:
		outValue[0] = HashRange(seed + 11, mins.x, maxs.x);
		if (parmCount > 1) {
			outValue[1] = HashRange(seed + 13, mins.y, maxs.y);
		}
		if (parmCount > 2) {
			outValue[2] = HashRange(seed + 15, mins.z, maxs.z);
		}
		switch (static_cast<int>(HashUnit(seed + 19) * 6.0f)) {
		case 0: outValue[0] = mins.x; break;
		case 1: outValue[0] = maxs.x; break;
		case 2: outValue[1] = mins.y; break;
		case 3: outValue[1] = maxs.y; break;
		case 4: outValue[2] = mins.z; break;
		default: outValue[2] = maxs.z; break;
		}
		return;
	case SPF_SPHERE_0:
	case SPF_SURFACE_SPHERE_0: {
		const idVec3 center((mins.x + maxs.x) * 0.5f, (mins.y + maxs.y) * 0.5f, (mins.z + maxs.z) * 0.5f);
		const idVec3 radius((maxs.x - mins.x) * 0.5f, (maxs.y - mins.y) * 0.5f, (maxs.z - mins.z) * 0.5f);
		idVec3 dir;
		SampleUnitSphere(seed, dir);
		float radial = 1.0f;
		if (spawnBase == SPF_SPHERE_0) {
			radial = HashUnit(seed + 21);
		}
		outValue[0] = center.x + radius.x * dir.x * radial;
		if (parmCount > 1) {
			outValue[1] = center.y + radius.y * dir.y * radial;
		}
		if (parmCount > 2) {
			outValue[2] = center.z + radius.z * dir.z * radial;
		}
		return;
	}
	case SPF_CYLINDER_0:
	case SPF_SURFACE_CYLINDER_0: {
		const float t = HashUnit(seed + 23);
		const float phi = HashRange(seed + 25, 0.0f, idMath::TWO_PI);
		const float radial = (spawnBase == SPF_SURFACE_CYLINDER_0) ? 1.0f : HashUnit(seed + 27);
		const float cx = LerpFloat(mins.x, maxs.x, t);
		const float cy = (mins.y + maxs.y) * 0.5f;
		const float cz = (mins.z + maxs.z) * 0.5f;
		const float ry = (maxs.y - mins.y) * 0.5f;
		const float rz = (maxs.z - mins.z) * 0.5f;
		outValue[0] = cx;
		if (parmCount > 1) {
			outValue[1] = cy + idMath::Cos(phi) * ry * radial;
		}
		if (parmCount > 2) {
			outValue[2] = cz + idMath::Sin(phi) * rz * radial;
		}
		return;
	}
	case SPF_SPIRAL_0: {
		const float x = HashRange(seed + 29, mins.x, maxs.x);
		const float range = (parms->mRange > BSE_TIME_EPSILON) ? parms->mRange : 1.0f;
		const float theta = idMath::TWO_PI * x / range;
		const float ry = HashRange(seed + 31, mins.y, maxs.y);
		const float rz = HashRange(seed + 33, mins.z, maxs.z);
		outValue[0] = x;
		if (parmCount > 1) {
			outValue[1] = idMath::Cos(theta) * ry;
		}
		if (parmCount > 2) {
			outValue[2] = idMath::Sin(theta) * rz;
		}
		return;
	}
	default:
		outValue[0] = HashRange(seed + 11, mins.x, maxs.x);
		if (parmCount > 1) {
			outValue[1] = HashRange(seed + 13, mins.y, maxs.y);
		}
		if (parmCount > 2) {
			outValue[2] = HashRange(seed + 15, mins.z, maxs.z);
		}
		return;
	}
}

ID_INLINE int GetSizeParmCount(int particleType) {
	switch (particleType) {
	case PTYPE_LINE:
	case PTYPE_ELECTRICITY:
	case PTYPE_LINKED:
	case PTYPE_ORIENTEDLINKED:
		return 1;
	case PTYPE_MODEL:
	case PTYPE_LIGHT:
	case PTYPE_DEBRIS:
		return 3;
	default:
		return 2;
	}
}

ID_INLINE void SetDrawVert(idDrawVert& vert, const idVec3& xyz, float s, float t, const byte rgba[4]) {
	vert.Clear();
	vert.xyz = xyz;
	vert.st[0] = s;
	vert.st[1] = t;
	vert.normal.Set(1.0f, 0.0f, 0.0f);
	vert.tangents[0].Set(0.0f, 1.0f, 0.0f);
	vert.tangents[1].Set(0.0f, 0.0f, 1.0f);
	vert.color[0] = rgba[0];
	vert.color[1] = rgba[1];
	vert.color[2] = rgba[2];
	vert.color[3] = rgba[3];
}

ID_INLINE void AppendQuad(srfTriangles_t* tri, const idVec3& p0, const idVec3& p1, const idVec3& p2, const idVec3& p3, const byte rgba[4]) {
	const int base = tri->numVerts;
	SetDrawVert(tri->verts[base + 0], p0, 0.0f, 0.0f, rgba);
	SetDrawVert(tri->verts[base + 1], p1, 1.0f, 0.0f, rgba);
	SetDrawVert(tri->verts[base + 2], p2, 1.0f, 1.0f, rgba);
	SetDrawVert(tri->verts[base + 3], p3, 0.0f, 1.0f, rgba);

	const int indexBase = tri->numIndexes;
	tri->indexes[indexBase + 0] = base + 0;
	tri->indexes[indexBase + 1] = base + 1;
	tri->indexes[indexBase + 2] = base + 3;
	tri->indexes[indexBase + 3] = base + 1;
	tri->indexes[indexBase + 4] = base + 2;
	tri->indexes[indexBase + 5] = base + 3;
	tri->numVerts += 4;
	tri->numIndexes += 6;
}
}

rvSegment::~rvSegment() {
	if (!mParticles) {
		return;
	}

	switch (mParticleType) {
	case PTYPE_LINE:
		delete[] static_cast<rvLineParticle*>(mParticles);
		break;
	case PTYPE_ORIENTED:
		delete[] static_cast<rvOrientedParticle*>(mParticles);
		break;
	case PTYPE_DECAL:
		delete[] static_cast<rvDecalParticle*>(mParticles);
		break;
	case PTYPE_MODEL:
		delete[] static_cast<rvModelParticle*>(mParticles);
		break;
	case PTYPE_LIGHT:
		delete[] static_cast<rvLightParticle*>(mParticles);
		break;
	case PTYPE_ELECTRICITY:
		delete[] static_cast<rvElectricityParticle*>(mParticles);
		break;
	case PTYPE_LINKED:
		delete[] static_cast<rvLinkedParticle*>(mParticles);
		break;
	case PTYPE_ORIENTEDLINKED:
		delete[] static_cast<sdOrientedLinkedParticle*>(mParticles);
		break;
	case PTYPE_DEBRIS:
		delete[] static_cast<rvDebrisParticle*>(mParticles);
		break;
	default:
		delete[] static_cast<rvSpriteParticle*>(mParticles);
		break;
	}

	mParticles = NULL;
	mUsedHead = NULL;
	mFreeHead = NULL;
}

rvSegmentTemplate* rvSegment::GetSegmentTemplate(void) {
	if (!mEffectDecl) {
		return NULL;
	}
	return (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle);
}

void rvSegment::ValidateSpawnRates() {
	mSecondsPerParticle.y = idMath::ClampFloat(0.002f, 300.0f, mSecondsPerParticle.y);
	mSecondsPerParticle.x = idMath::ClampFloat(mSecondsPerParticle.y, 300.0f, mSecondsPerParticle.x);
}

void rvSegment::GetSecondsPerParticle(rvBSE* effect, rvSegmentTemplate* st, rvParticleTemplate* pt) {
	if (st->mDensity.y == 0.0f) {
		mCount = st->mCount;
	}
	else {
		float volume = Max(1.0f, mEffectDecl ? mEffectDecl->GetSize() * 0.025f : 1.0f);
		volume = idMath::ClampFloat(0.002f, 2048.0f, volume);
		mCount.x = st->mDensity.x * volume;
		mCount.y = st->mDensity.y * volume;
		if (st->mParticleCap > 0.0f) {
			mCount.x = idMath::ClampFloat(1.0f, st->mParticleCap, mCount.x);
			mCount.y = idMath::ClampFloat(1.0f, st->mParticleCap, mCount.y);
		}
	}

	if (st->mSegType == SEG_EMITTER || st->mSegType == SEG_TRAIL) {
		if (mCount.x > 0.0f) {
			mSecondsPerParticle.x = 1.0f / mCount.x;
		}
		if (mCount.y > 0.0f) {
			mSecondsPerParticle.y = 1.0f / mCount.y;
		}
		ValidateSpawnRates();
	}
}

void rvSegment::InitTime(rvBSE* effect, rvSegmentTemplate* st, float time) {
	SetExpired(false);
	mSegStartTime = rvRandom::flrand(st->mLocalStartTime.x, st->mLocalStartTime.y) + time;
	mSegEndTime = rvRandom::flrand(st->mLocalDuration.x, st->mLocalDuration.y) + mSegStartTime;
	mLastTime = mSegStartTime;
	if (!st->GetIgnoreDuration() || (!effect->GetLooping() && !st->GetSoundLooping())) {
		effect->SetDuration(mSegEndTime - time);
	}
}

float rvSegment::AttenuateDuration(rvBSE* effect, rvSegmentTemplate* st) {
	return effect->GetAttenuation(st) * (mSegEndTime - mSegStartTime);
}

float rvSegment::AttenuateInterval(rvBSE* effect, rvSegmentTemplate* st) {
	float interval = mSecondsPerParticle.x + (mSecondsPerParticle.y - mSecondsPerParticle.x);
	interval = idMath::ClampFloat(mSecondsPerParticle.y, mSecondsPerParticle.x, interval);
	if (!st->GetAttenuateEmitter()) {
		return interval;
	}

	float attenuation = effect->GetAttenuation(st);
	if (st->GetInverseAttenuate()) {
		attenuation = 1.0f - attenuation;
	}
	if (attenuation < 0.002f) {
		return 1.0f;
	}
	return Max(1.1920929e-7f, interval / attenuation);
}

float rvSegment::AttenuateCount(rvBSE* effect, rvSegmentTemplate* st, float min, float max) {
	float count = rvRandom::flrand(min, max);
	count = idMath::ClampFloat(min, max, count);
	if (!st->GetAttenuateEmitter()) {
		return count;
	}
	float attenuation = effect->GetAttenuation(st);
	if (st->GetInverseAttenuate()) {
		attenuation = 1.0f - attenuation;
	}
	return attenuation * count;
}

void rvSegment::RefreshParticles(rvBSE* effect, rvSegmentTemplate* st) {
	if (!effect || !st || !st->mParticleTemplate.UsesEndOrigin()) {
		return;
	}

	for (rvParticle* p = mUsedHead; p; p = p->GetNext()) {
		p->Refresh(effect, st, &st->mParticleTemplate);
	}
}

void rvSegment::Init(rvBSE* effect, const rvDeclEffect* effectDecl, int segmentTemplateHandle, float time) {
	mSegmentTemplateHandle = segmentTemplateHandle;
	mFlags = 0;
	mEffectDecl = effectDecl;
	mSurfaceIndex = -1;
	mParticleCount = 0;
	mLoopParticleCount = 0;
	mParticles = NULL;
	mUsedHead = NULL;
	mFreeHead = NULL;
	mActiveCount = 0;
	mLastTime = time;
	mSecondsPerParticle.Zero();
	mCount.Set(1.0f, 1.0f);
	mSoundVolume = 0.0f;
	mFreqShift = 1.0f;

	const rvSegmentTemplate* st = effectDecl->GetSegmentTemplate(segmentTemplateHandle);
	if (!st) {
		mParticleType = PTYPE_NONE;
		return;
	}

	mParticleType = st->mParticleTemplate.GetType();
	InitTime(effect, const_cast<rvSegmentTemplate*>(st), time);
	GetSecondsPerParticle(effect, const_cast<rvSegmentTemplate*>(st), const_cast<rvParticleTemplate*>(&st->mParticleTemplate));
}

void rvSegment::ResetTime(rvBSE* effect, float time) {
	rvSegmentTemplate* st = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle);
	if (st && !st->GetInfiniteDuration()) {
		InitTime(effect, st, time);
	}

}

void rvSegment::InitParticles(rvBSE* effect) {
	if (!effect) {
		return;
	}

	InitParticleArray(effect);
	mActiveCount = 0;
}

bool rvSegment::Check(rvBSE* effect, float time, float offset) {
	rvSegmentTemplate* st = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle);
	if (!st || !st->GetEnabled()) {
		return false;
	}
	if (time < mSegStartTime) {
		return false;
	}

	const bool infinite = st->GetInfiniteDuration() || st->GetSoundLooping();
	if (!infinite && time > (mSegEndTime + BSE_TIME_EPSILON)) {
		if (GetSoundPlaying() && effect->GetReferenceSound()) {
			effect->GetReferenceSound()->StopSound(static_cast<s_channelType>(mSegmentTemplateHandle + SCHANNEL_ONE));
			SetSoundPlaying(false);
		}
		SetExpired(true);
		return false;
	}

	switch (st->mSegType) {
	case SEG_EMITTER:
	case SEG_TRAIL: {
		if (!st->GetHasParticles()) {
			return true;
		}

		const float spawnEnd = infinite ? (time + BSE_FUTURE) : Min(time + BSE_FUTURE, mSegEndTime);
		if (spawnEnd <= mSegStartTime + BSE_TIME_EPSILON) {
			return true;
		}

		float interval = AttenuateInterval(effect, st);
		interval = Max(BSE_TIME_EPSILON, interval);

		float spawnTime = Max(mLastTime, mSegStartTime);
		if (spawnTime > mSegStartTime) {
			const float relative = spawnTime - mSegStartTime;
			const int steps = Max(0, static_cast<int>(idMath::Floor(relative / interval)));
			spawnTime = mSegStartTime + steps * interval;
			while (spawnTime + BSE_TIME_EPSILON < Max(mLastTime, mSegStartTime)) {
				spawnTime += interval;
			}
		}

		const int maxSpawn = Max(1, effect->GetLooping() ? mLoopParticleCount : mParticleCount);
		int spawned = 0;
		while (spawnTime <= spawnEnd + BSE_TIME_EPSILON && spawned < maxSpawn) {
			if (st->mSegType == SEG_TRAIL) {
				const idVec3 localOffset = effect->GetCurrentAxisTransposed() * effect->GetInterpolatedOffset(spawnTime);
				SpawnParticle(effect, st, spawnTime, localOffset, mat3_identity);
			}
			else {
				SpawnParticle(effect, st, spawnTime, vec3_origin, mat3_identity);
			}
			spawnTime += interval;
			++spawned;
		}

		mLastTime = Max(mLastTime, time);
		return true;
	}
	case SEG_SPAWNER:
		if (!GetExpired() && st->GetHasParticles()) {
			const int count = idMath::ClampInt(
				0,
				MAX_PARTICLES,
				static_cast<int>(idMath::Ceil(AttenuateCount(effect, st, mCount.x, mCount.y))));
			if (count > 0) {
				SpawnParticles(effect, st, time, count);
			}
			SetExpired(true);
		}
		return true;
	case SEG_EFFECT:
		if (!GetExpired() && st->mNumEffects > 0 && game) {
			const int index = rvRandom::irand(0, st->mNumEffects - 1);
			const rvDeclEffect* nested = st->mEffects[index];
			if (nested) {
				game->PlayEffect(
					nested,
					effect->GetCurrentOrigin(),
					effect->GetCurrentAxis(),
					false,
					effect->GetHasEndOrigin() ? effect->GetCurrentEndOrigin() : vec3_origin,
					false,
					false,
					EC_IGNORE,
					vec4_one);
			}
			SetExpired(true);
		}
		return true;
	case SEG_SOUND:
		if (effect->GetReferenceSound() && st->mSoundShader) {
			if (!GetSoundPlaying()) {
				effect->GetReferenceSound()->StartSound(
					st->mSoundShader,
					static_cast<s_channelType>(mSegmentTemplateHandle + SCHANNEL_ONE),
					0.0f,
					0,
					false);
				SetSoundPlaying(true);
			}
			mSoundVolume = st->GetSoundVolume();
			mFreqShift = st->GetFreqShift();
			effect->UpdateSoundEmitter(st, this);
		}
		return true;
	case SEG_DECAL:
		if (!GetExpired()) {
			CreateDecal(effect, time);
			SetExpired(true);
		}
		return true;
	case SEG_LIGHT:
		return HandleLight(effect, st, time);
	default:
		return true;
	}
}

bool rvSegment::UpdateParticles(rvBSE* effect, float time) {
	rvSegmentTemplate* st = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle);
	if (!st || !st->GetEnabled()) {
		return false;
	}

	Handle(effect, time);

	if (st->GetInfiniteDuration() || st->GetSmoker() || st->GetHasPhysics() || st->mParticleTemplate.GetNumTimeoutEffects() > 0) {
		UpdateGenericParticles(effect, st, time);
	}
	else {
		UpdateSimpleParticles(time);
	}

	return mUsedHead != NULL;
}

void rvSegment::CalcCounts(rvBSE* effect, float time) {
	mParticleCount = 0;
	mLoopParticleCount = 0;

	rvSegmentTemplate* st = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle);
	if (!st || !st->GetHasParticles()) {
		return;
	}

	const float segmentDuration = Max(0.0f, mSegEndTime - mSegStartTime);
	const float particleDuration = st->mParticleTemplate.GetMaxDuration() + 0.016f;
	const float effectiveDuration = Max(segmentDuration, particleDuration);

	int count = 0;
	switch (st->mSegType) {
	case SEG_SPAWNER:
		count = idMath::ClampInt(0, MAX_PARTICLES, static_cast<int>(idMath::Ceil(mCount.y)));
		break;
	case SEG_EMITTER:
	case SEG_TRAIL:
		if (mSecondsPerParticle.y > BSE_TIME_EPSILON) {
			count = idMath::ClampInt(
				0,
				MAX_PARTICLES,
				static_cast<int>(idMath::Ceil(effectiveDuration / mSecondsPerParticle.y)) + 1);
		}
		break;
	case SEG_DECAL:
	case SEG_LIGHT:
		count = 1;
		break;
	default:
		break;
	}

	mParticleCount = count;
	mLoopParticleCount = count;

	if (!effect->GetLooping()) {
		effect->SetDuration((mSegEndTime - time) + st->mParticleTemplate.GetMaxDuration());
	}

}

void rvSegment::AllocateSurface(rvBSE* effect, idRenderModel* model) {
	mSurfaceIndex = -1;
	if (!effect || !model) {
		return;
	}

	rvSegmentTemplate* st = GetSegmentTemplate();
	if (!st || !st->GetEnabled() || !st->GetHasParticles()) {
		return;
	}

	rvParticleTemplate* pt = st->GetParticleTemplate();
	if (!pt || pt->GetType() == PTYPE_NONE) {
		return;
	}

	const int particleCount = effect->GetLooping() ? mLoopParticleCount : mParticleCount;
	if (particleCount <= 0) {
		return;
	}

	const int maxVerts = idMath::ClampInt(4, 10000, particleCount * pt->GetVertexCount());
	const int maxIndexes = idMath::ClampInt(6, 30000, particleCount * pt->GetIndexCount());
	srfTriangles_t* tri = model->AllocSurfaceTriangles(maxVerts, maxIndexes);
	if (!tri) {
		return;
	}
	tri->numVerts = 0;
	tri->numIndexes = 0;

	modelSurface_t surf;
	surf.id = 0;
	surf.geometry = tri;
	surf.shader = pt->GetMaterial() ? pt->GetMaterial() : declManager->FindMaterial("_default");
	model->AddSurface(surf);
	mSurfaceIndex = model->NumSurfaces() - 1;

	const bool motionTrail = (pt->GetTrailType() == TRAIL_MOTION) && (pt->GetMaxTrailCount() > 0 || pt->GetMaxTrailTime() >= BSE_TIME_EPSILON);
	if (motionTrail) {
		const int trailVerts = idMath::ClampInt(4, 10000, particleCount * pt->GetMaxTrailCount() * 2 + 2);
		const int trailIndexes = idMath::ClampInt(6, 30000, particleCount * pt->GetMaxTrailCount() * 12);
		srfTriangles_t* trailTri = model->AllocSurfaceTriangles(trailVerts, trailIndexes);
		if (trailTri) {
			trailTri->numVerts = 0;
			trailTri->numIndexes = 0;

			modelSurface_t trailSurf;
			trailSurf.id = 0;
			trailSurf.geometry = trailTri;
			trailSurf.shader = pt->GetTrailMaterial() ? pt->GetTrailMaterial() : surf.shader;
			model->AddSurface(trailSurf);
			SetHasMotionTrail(true);
		}
	}
}

void rvSegment::CreateDecal(rvBSE* effect, float time) {
	if (!effect || !session || !session->rw) {
		return;
	}

	rvSegmentTemplate* st = GetSegmentTemplate();
	if (!st || !st->GetHasParticles()) {
		return;
	}

	rvParticleTemplate* pt = st->GetParticleTemplate();
	if (!pt || !pt->GetMaterial()) {
		return;
	}

	float sizeParms[3] = { 32.0f, 32.0f, 8.0f };
	if (pt->mpSpawnSize) {
		pt->mpSpawnSize->Spawn(sizeParms, *pt->mpSpawnSize, NULL, NULL);
	}

	sizeParms[0] = Max(1.0f, idMath::Fabs(sizeParms[0]));
	sizeParms[1] = Max(1.0f, idMath::Fabs(sizeParms[1]));
	sizeParms[2] = Max(1.0f, idMath::Fabs(sizeParms[2]));

	float rotateParms[3] = { 0.0f, 0.0f, 0.0f };
	if (pt->mpSpawnRotate) {
		pt->mpSpawnRotate->Spawn(rotateParms, *pt->mpSpawnRotate, NULL, NULL);
	}
	const float rotateRad = rotateParms[0] * idMath::TWO_PI;

	idVec3 origin = effect->GetCurrentOrigin();
	idMat3 axis = effect->GetCurrentAxis();

	idVec3 normal = axis[0];
	idVec3 right = axis[1];
	idVec3 up = axis[2];

	if (rotateRad != 0.0f) {
		const float s = idMath::Sin(rotateRad);
		const float c = idMath::Cos(rotateRad);
		const idVec3 rotatedRight = right * c - up * s;
		const idVec3 rotatedUp = right * s + up * c;
		right = rotatedRight;
		up = rotatedUp;
	}

	const idVec3 halfRight = right * (sizeParms[0] * 0.5f);
	const idVec3 halfUp = up * (sizeParms[1] * 0.5f);

	idFixedWinding winding;
	winding.Clear();
	winding += origin + halfRight + halfUp;
	winding += origin - halfRight + halfUp;
	winding += origin - halfRight - halfUp;
	winding += origin + halfRight - halfUp;

	const idVec3 projectionOrigin = origin - normal * (sizeParms[2] * 0.5f);
	const int startTimeMs = static_cast<int>(time * 1000.0f);
	session->rw->ProjectDecalOntoWorld(
		winding,
		projectionOrigin,
		false,
		sizeParms[2],
		pt->GetMaterial(),
		startTimeMs);
}

bool rvSegment::GetLocked(void) {
	rvSegmentTemplate* st = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle);
	return st ? st->GetLocked() : false;
}

void rvSegment::Handle(rvBSE* effect, float time) {
	rvSegmentTemplate* st = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle);
	if (!st || time < mSegStartTime) {
		return;
	}

	switch (st->mSegType) {
	case SEG_EMITTER:
	case SEG_SPAWNER:
		if (effect->GetEndOriginChanged()) {
			RefreshParticles(effect, st);
		}
		break;
	case SEG_SOUND:
		effect->UpdateSoundEmitter(st, this);
		break;
	case SEG_LIGHT:
		if (st->GetEnabled()) {
			HandleLight(effect, st, time);
		}
		break;
	default:
		break;
	}
}

bool rvSegment::Active() {
	rvSegmentTemplate* st = GetSegmentTemplate();
	if (!st || !st->GetEnabled() || !st->GetHasParticles()) {
		return false;
	}
	return mUsedHead != NULL;
}

void rvSegment::InitLight(rvBSE* effect, rvSegmentTemplate* st, float time) {
	if (!effect || !st || !st->GetHasParticles()) {
		return;
	}
	if (!mUsedHead) {
		SpawnParticle(effect, st, time);
	}
	if (mUsedHead) {
		mUsedHead->InitLight(effect, st, time);
		mActiveCount = 1;
	}
}

bool rvSegment::HandleLight(rvBSE* effect, rvSegmentTemplate* st, float time) {
	if (!effect || !st || !st->GetEnabled()) {
		return false;
	}
	if (!mUsedHead) {
		InitLight(effect, st, time);
		if (!mUsedHead) {
			return false;
		}
	}

	const bool infinite = st->GetInfiniteDuration() || st->GetSoundLooping();
	const bool presented = mUsedHead->PresentLight(effect, st->GetParticleTemplate(), time, infinite);

	if (!infinite && mUsedHead && mUsedHead->GetEndTime() - BSE_TIME_EPSILON <= time) {
		rvParticle* expired = mUsedHead;
		expired->Destroy();
		expired->SetNext(mFreeHead);
		mFreeHead = expired;
		mUsedHead = NULL;
		mActiveCount = 0;
		return false;
	}

	return presented;
}

void rvSegment::ClearSurface(rvBSE* effect, idRenderModel* model) {
	if (!model || mSurfaceIndex < 0 || mSurfaceIndex >= model->NumSurfaces()) {
		return;
	}

	srfTriangles_t* tri = model->Surface(mSurfaceIndex)->geometry;
	if (tri) {
		tri->numVerts = 0;
		tri->numIndexes = 0;
	}

	if (GetHasMotionTrail() && mSurfaceIndex + 1 < model->NumSurfaces()) {
		srfTriangles_t* trailTri = model->Surface(mSurfaceIndex + 1)->geometry;
		if (trailTri) {
			trailTri->numVerts = 0;
			trailTri->numIndexes = 0;
		}
	}
}

void rvSegment::RenderTrail(rvBSE* effect, const renderEffect_s* owner, idRenderModel* model, float time) {
	rvSegmentTemplate* st = GetSegmentTemplate();
	if (!st || !GetHasMotionTrail()) {
		return;
	}
	rvParticleTemplate* pt = st->GetParticleTemplate();
	if (!pt || pt->GetTrailType() != TRAIL_MOTION) {
		return;
	}
	RenderMotion(effect, owner, model, pt, time);
}

void rvSegment::Render(rvBSE* effect, const renderEffect_s* owner, idRenderModel* model, float time) {
	if (!effect || !owner || !model) {
		return;
	}

	rvSegmentTemplate* st = GetSegmentTemplate();
	if (!st || !st->GetEnabled() || !st->GetHasParticles() || mSurfaceIndex < 0 || mSurfaceIndex >= model->NumSurfaces()) {
		return;
	}

	rvParticleTemplate* pt = st->GetParticleTemplate();
	if (!pt) {
		return;
	}

	srfTriangles_t* tri = model->Surface(mSurfaceIndex)->geometry;
	if (!tri) {
		return;
	}

	const int maxVerts = tri->numAllocedVerts > 0 ? tri->numAllocedVerts : 0;
	const int maxIndexes = tri->numAllocedIndices > 0 ? tri->numAllocedIndices : 0;
	if (maxVerts <= 0 || maxIndexes <= 0) {
		return;
	}
	tri->numVerts = 0;
	tri->numIndexes = 0;

	idMat3 view;
	const idMat3 ownerAxisTranspose = owner->axis.Transpose();
	view[1] = ownerAxisTranspose * effect->GetViewAxis()[1];
	view[2] = ownerAxisTranspose * effect->GetViewAxis()[2];
	view[0] = ownerAxisTranspose * (effect->GetViewOrg() - owner->origin);

	int rendered = 0;
	const int particleType = pt->GetType();
	const bool linkedStrip = (particleType == PTYPE_LINKED || particleType == PTYPE_ORIENTEDLINKED);
	const bool depthSort = st->GetDepthSort() && !linkedStrip;
	const bool inverseOrder = st->GetInverseDrawOrder();

	auto renderParticle = [&](rvParticle* p) -> bool {
		if (!p) {
			return false;
		}
		if (st->GetInfiniteDuration()) {
			// Vanilla behavior keeps looping segment particles renderable by
			// continuously pushing end-time forward each frame.
			p->ExtendLife(time + 1.0f);
		}

		int requiredIndexes = pt->GetIndexCount();
		if (linkedStrip && tri->numVerts == 0) {
			requiredIndexes = 0;
		}

		if (tri->numVerts + pt->GetVertexCount() > maxVerts || tri->numIndexes + requiredIndexes > maxIndexes) {
			return false;
		}
		if (p->Render(effect, pt, view, tri, time, 1.0f)) {
			++rendered;
			if (pt->GetTrailType() == TRAIL_BURN) {
				p->RenderBurnTrail(effect, pt, view, tri, time);
			}
		}
		return true;
	};

	if (!depthSort && !inverseOrder) {
		for (rvParticle* p = mUsedHead; p; p = p->GetNext()) {
			if (!renderParticle(p)) {
				break;
			}
		}
	}
	else {
		rvParticle* drawList[kMaxVisualParticles];
		int drawCount = 0;
		for (rvParticle* p = mUsedHead; p; p = p->GetNext()) {
			if (drawCount >= kMaxVisualParticles) {
				break;
			}
			drawList[drawCount++] = p;
		}

		if (depthSort) {
			const idVec3 eyePos = view[0];
			for (int i = 1; i < drawCount; ++i) {
				rvParticle* key = drawList[i];
				const float keyDist = key->UpdateViewDist(eyePos);
				int j = i - 1;

				while (j >= 0) {
					const float curDist = drawList[j]->UpdateViewDist(eyePos);
					if (curDist >= keyDist) {
						break;
					}
					drawList[j + 1] = drawList[j];
					--j;
				}

				drawList[j + 1] = key;
			}
		}

		if (inverseOrder) {
			for (int i = drawCount - 1; i >= 0; --i) {
				if (!renderParticle(drawList[i])) {
					break;
				}
			}
		}
		else {
			for (int i = 0; i < drawCount; ++i) {
				if (!renderParticle(drawList[i])) {
					break;
				}
			}
		}
	}
	R_BoundTriSurf(tri);
	mActiveCount = rendered;
}

#if 0
void rvSegment::Handle(rvBSE* effect, float time)
{
	rvSegmentTemplate* v3; // edx

	v3 = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(this->mSegmentTemplateHandle);
	if (v3 && this->mSegStartTime <= (double)time)
	{
		switch (v3->mSegType)
		{
		case SEG_EMITTER:
		case SEG_SPAWNER:
			if (effect->GetEndOriginChanged())
				RefreshParticles(effect, v3);
			break;
		case SEG_SOUND:
			effect->UpdateSoundEmitter(v3, this);
			break;
		case SEG_LIGHT:
			if ((v3->mFlags & 1) != 0)
				HandleLight(effect, v3, time);
			break;
		default:
			return;
		}
	}
}

void rvSegment::ValidateSpawnRates()
{
	double v1; // st6
	double v2; // st7
	double v3; // st6
	float v4; // [esp+0h] [ebp-4h]
	float v5; // [esp+0h] [ebp-4h]
	float v6; // [esp+0h] [ebp-4h]
	float v7; // [esp+0h] [ebp-4h]

	v1 = this->mSecondsPerParticle.y;
	if (v1 >= 0.0020000001)
	{
		v2 = 300.0;
		if (v1 > 300.0)
		{
			v6 = 300.0;
			goto LABEL_4;
		}
	}
	else
	{
		v1 = 0.0020000001;
		v2 = 300.0;
	}
	v6 = v1;
LABEL_4:
	v3 = v6;
	this->mSecondsPerParticle.y = v6;
	v4 = this->mSecondsPerParticle.x;
	if (v4 < v3 || (v3 = v4, v4 <= v2))
	{
		v5 = v3;
		this->mSecondsPerParticle.x = v5;
	}
	else
	{
		v7 = v2;
		this->mSecondsPerParticle.x = v7;
	}
}

void rvSegment::GetSecondsPerParticle(rvBSE* effect, rvSegmentTemplate* st, rvParticleTemplate* pt)
{
	double v4; // st7
	double v7; // st7
	int v8; // edi
	float volume; // [esp+10h] [ebp+8h]
	float volumea; // [esp+10h] [ebp+8h]

	v4 = 0.0;
	if (0.0 == st->mDensity.y)
	{
		this->mCount = st->mCount;
	}
	else
	{
		volume = pt->GetSpawnVolume(effect);
		v7 = 0.0020000001;
		if (volume >= 0.0020000001)
		{
			v7 = volume;
			if (volume > 2048.0)
				v7 = 2048.0;
		}
		volumea = v7;
		this->mCount.x = st->mDensity.x * volumea;
		this->mCount.y = volumea * st->mDensity.y;
		v4 = 0.0;
	}
	v8 = st->mSegType;
	if (v8 == 2 || v8 == 4)
	{
		if (v4 != this->mCount.x)
			this->mSecondsPerParticle.x = 1.0 / this->mCount.x;
		if (v4 != this->mCount.y)
			this->mSecondsPerParticle.y = 1.0 / this->mCount.y;
		ValidateSpawnRates();
	}
}

void rvSegment::InitTime(rvBSE* effect, rvSegmentTemplate* st, float time)
{
	float sta; // [esp+20h] [ebp+8h]
	float timea; // [esp+24h] [ebp+Ch]

	this->mFlags &= 0xFFFFFFFE;
	this->mSegStartTime = rvRandom::flrand(st->mLocalStartTime.x, st->mLocalStartTime.y) + time;
	sta = rvRandom::flrand(st->mLocalDuration.x, st->mLocalDuration.y);
	this->mSegEndTime = this->mSegStartTime + sta;
	if ((st->mFlags & 0x10) == 0 || (effect->GetLooping()) == 0 && !st->GetSoundLooping())
	{
		timea = this->mSegEndTime - time;
		effect->SetDuration(timea);
	}
}

float rvSegment::AttenuateDuration(rvBSE* effect, rvSegmentTemplate* st)
{
	return (float)(effect->GetAttenuation(st) * (this->mSegEndTime - this->mSegStartTime));
}

float rvSegment::AttenuateInterval(rvBSE* effect, rvSegmentTemplate* st)
{
	double result; // st7
	float v5; // [esp+4h] [ebp-Ch]
	float v6; // [esp+8h] [ebp-8h]
	float v7; // [esp+Ch] [ebp-4h]
	float atten; // [esp+18h] [ebp+8h]
	float attena; // [esp+18h] [ebp+8h]
	float attenb; // [esp+18h] [ebp+8h]

	v5 = (this->mSecondsPerParticle.y - this->mSecondsPerParticle.x) * 1.0f; // bse_detailLevel.internalVar->floatValue
		+ this->mSecondsPerParticle.x;
	v6 = this->mSecondsPerParticle.y;
	v7 = this->mSecondsPerParticle.x;
	if (v6 <= (double)v5)
	{
		if (v7 < (double)v5)
			v5 = v7;
	}
	else
	{
		v5 = v6;
	}
	if ((st->mFlags & 0x40) == 0)
		return v5;
	atten = effect->GetAttenuation(st);
	if ((st->mFlags & 0x80) != 0)
		atten = 1.0 - atten;
	if (atten < 0.0020000001)
		return 1.0;
	attena = v5 / atten;
	result = attena;
	if (attena <= 0.00000011920929)
	{
		attenb = 0.00000011920929 + 0.00000011920929;
		result = attenb;
	}
	return result;
}

float rvSegment::AttenuateCount(rvBSE* effect, rvSegmentTemplate* st, float min, float max)
{
	double v5; // st7
	float v7; // [esp+8h] [ebp-4h]
	float v8; // [esp+8h] [ebp-4h]
	float atten; // [esp+18h] [ebp+Ch]

	v7 = (max - min) * /*bse_detailLevel.internalVar->floatValue*/ 1.0f + min;
	v8 = rvRandom::flrand(min, v7);
	if (min <= (double)v8)
	{
		if (max < (double)v8)
			v8 = max;
	}
	else
	{
		v8 = min;
	}
	if ((st->mFlags & 0x40) != 0)
	{
		atten = effect->GetAttenuation(st);
		v5 = atten;
		if ((st->mFlags & 0x80) != 0)
			v5 = 1.0 - v5;
		v8 = v5 * v8;
	}
	return v8;
}

void rvSegment::RefreshParticles(rvBSE* effect, rvSegmentTemplate* st)
{
	rvParticle* v4; // ecx
	rvParticle* v5; // esi

	if (st->mParticleTemplate.UsesEndOrigin())
	{
		v4 = this->mUsedHead;
		if (v4)
		{
			do
			{
				v5 = v4->GetNext();
				v4->Refresh(effect, st, &st->mParticleTemplate);					
				v4 = v5;
			} while (v5);
		}
	}
}

void rvParticle::DoRenderBurnTrail(rvBSE* effect, rvParticleTemplate* pt, const idMat3& view, srfTriangles_t* tri, float time)
{
	int v7; // ecx
	int v8; // edi
	double v10; // st7
	float v11; // [esp+0h] [ebp-20h]
	int delta; // [esp+18h] [ebp-8h]
	float v13; // [esp+1Ch] [ebp-4h]
	float trailTime; // [esp+30h] [ebp+10h]
	float trailTimea; // [esp+30h] [ebp+10h]

	v7 = this->mTrailCount;
	if (v7)
	{
		if (0.0 != this->mTrailTime)
		{
			v8 = 1;
			delta = 1;
			v13 = this->mTrailTime / (double)v7;
			if (v7 + 1 > 1)
			{
				do
				{
					trailTime = time - (double)delta * v13;
					v10 = trailTime;
					if (this->mStartTime <= (double)trailTime && this->mEndTime > v10)
					{
						trailTimea = (double)(this->mTrailCount - v8) / (double)this->mTrailCount;
						v11 = v10;
						Render(effect, pt, view, tri, v11, trailTimea);
					}
					delta = ++v8;
				} while (v8 < this->mTrailCount + 1);
			}
		}
	}
}

void rvSegment::RenderMotion(rvBSE* effect, const renderEffect_s* owner, idRenderModel* model, rvParticleTemplate* pt, float time)
{
	const modelSurface_t* v7; // eax
	rvParticle* v8; // esi
	srfTriangles_t* v9; // ebx
	modelSurface_t* surf; // [esp+28h] [ebp+Ch]

	v7 = model->Surface(this->mSurfaceIndex + 1);
	v8 = this->mUsedHead;
	v9 = v7->geometry;
	surf = (modelSurface_t*)v7;
	if (v8)
	{
		do
		{
			v8->RenderMotion(effect, pt, v9, owner, time, pt->GetTrailScale());
			v8 = v8->GetNext();
		} while (v8);
		v7 = surf;
	}
	R_BoundTriSurf(v7->geometry);
}

rvParticle* rvSegment::SpawnParticle(rvBSE* effect, rvSegmentTemplate* st, float birthTime, const idVec3& initOffset, const idMat3& initAxis)
{
	rvParticle* v6; // esi
	rvParticle* v7; // edx
	float initAxisa; // [esp+28h] [ebp+14h]

	if ((mEffectDecl->GetSegmentTemplate(this->mSegmentTemplateHandle)->mFlags & 0x100) != 0)
	{
		v6 = this->mParticles;
	}
	else
	{
		v6 = this->mFreeHead;
		if (!v6)
			return v6;
		v7 = this->mUsedHead;
		this->mFreeHead = v6->GetNext();
		v6->SetNext(v7);
		this->mUsedHead = v6;
	}
	if (v6)
	{
		initAxisa = birthTime - effect->GetStartTime();
		v6->FinishSpawn(effect, this, birthTime, initAxisa, initOffset, initAxis);		
	}
	return v6;
}

void rvSegment::SpawnParticles(rvBSE* effect, rvSegmentTemplate* st, float birthTime, int count)
{
	int v6; // ebp
	rvParticle* v8; // ecx
	rvParticle* v9; // eax
	int v11; // [esp+20h] [ebp-4h]
	float counta; // [esp+34h] [ebp+10h]

	v6 = 0;
	v11 = 0;
	if (count > 0)
	{
		while ((this->mEffectDecl->GetSegmentTemplate(this->mSegmentTemplateHandle)->mFlags & 0x100) == 0)
		{
			v8 = this->mFreeHead;
			if (v8)
			{
				v9 = this->mUsedHead;
				this->mFreeHead = v8->GetNext();
				v8->SetNext(v9);
				this->mUsedHead = v8;
			LABEL_6:
				if (v8)
				{
					if (count == 1)
					{
						v8->FinishSpawn(effect, this, birthTime, 0.0, vec3_origin, mat3_identity);						
					}
					else
					{
						counta = (double)v11 / (double)(count - 1);
						v8->FinishSpawn(effect, this, birthTime, counta, vec3_origin, mat3_identity);						
					}
				}
			}
			v11 = ++v6;
			if (v6 >= count)
				return;
		}
		v8 = this->mParticles;
		goto LABEL_6;
	}
}

void rvSegment::PlayEffect(rvBSE* effect, rvSegmentTemplate* st, float depthOffset)
{
	int v4; // eax
	idDeclBase* v5; // ecx
	int v6; // eax

	v4 = st->mNumEffects;
	if (v4)
	{
		v5 = st->mEffects[rvRandom::irand(0, v4 - 1)]->base;
		v6 = v5->Index();
// jmarshall - todo add PlayEffect to game code.
		//((void(__thiscall*)(idGame*, int, idVec3*, idVec3*, idMat3*, _DWORD, idVec3*, _DWORD))game->PlayEffect)(
		//	game,
		//	v6,
		//	&effect->mMaterialColor,
		//	&effect->mCurrentOrigin,
		//	&effect->mCurrentAxis,
		//	0,
		//	&vec3_origin,
		//	LODWORD(depthOffset));
		common->FatalError("rvSegment::PlayEffect\n");
// jmarshall end
	}
}

void rvSegment::UpdateSimpleParticles(float time)
{
	rvDeclEffect* v3; // eax
	int v4; // ecx
	rvParticle* v5; // esi
	rvParticle* v6; // ebp
	rvParticle* v7; // ebx
	float pt; // [esp+14h] [ebp-8h]

	v3 = (rvDeclEffect *)this->mEffectDecl;
	v4 = this->mSegmentTemplateHandle;
	v5 = this->mUsedHead;
	v6 = 0;
	this->mActiveCount = 0;
	if (v5)
	{
		do
		{
			v7 = v5->GetNext();
			pt = v5->GetEndTime() - 0.002000000094994903;
			if (pt > (double)time)
			{
				v6 = v5;
				this->mActiveCount += v5->Update(&v3->GetSegmentTemplate(v4)->mParticleTemplate, time);
			}
			else
			{
				if (v6)
					v6->SetNext(v7);
				else
					this->mUsedHead = v7;
				rvParticle* destroyedParticle = v5; // jmarshall: should be fine, eval if broken, passed a function here?
				v5->SetNext(this->mFreeHead);
				destroyedParticle->Destroy();
				this->mFreeHead = v5;
			}
			v5 = v7;
		} while (v7);
	}
}

void rvSegment::UpdateGenericParticles(rvBSE* effect, rvSegmentTemplate* st, float time)
{
	rvSegment* v5; // esi
	unsigned int v6; // eax
	rvParticle* v7; // esi
	char v8; // al
	bool v10; // bl
	int v11; // eax
	bool(__thiscall * v12)(rvParticle*); // eax
	bool v13; // [esp+17h] [ebp-Dh]
	rvParticle* v15; // [esp+1Ch] [ebp-8h]
	float v16; // [esp+20h] [ebp-4h]
	rvParticle* next; // [esp+28h] [ebp+4h]
	char infinite; // [esp+2Ch] [ebp+8h]

	v5 = this;
	v13 = st->GetSmoker();
	v6 = (unsigned int)st->mFlags >> 5;
	v5->mActiveCount = 0;
	v7 = v5->mUsedHead;
	v8 = v6 & 1;
	infinite = v8;
	v15 = 0;
	if (v7)
	{
		while (1)
		{
			v10 = 0;
			next = v7->GetNext();
			if (v8)
			{
				v7->RunPhysics(effect, st, time);
				if (effect->GetStopped())
					goto LABEL_9;
			}
			else
			{
				v16 = v7->GetEndTime() - 0.002000000094994903;
				if (v16 <= (double)time)
				{
					v7->CheckTimeoutEffect(effect, st, time);
				LABEL_9:
					v10 = 1;
					goto LABEL_10;
				}
				v10 = v7->RunPhysics(effect, st, time);
			}
		LABEL_10:
			if (effect->GetStopped() && (v7->GetFlags() & 0x200000) == 0)
				v10 = 1;
			if (v13)
			{
				v11 = st->mTrailSegmentIndex;
				if (v11 >= 0)
					v7->EmitSmokeParticles(effect, effect->GetTrailSegment(v11), &st->mParticleTemplate, time);
			}
			if (v10)
			{
				if (v15)
					v15->SetNext(next);
				else
					this->mUsedHead = next;
				//v12 = v7->Destroy;
				rvParticle* destroyedParticle = v7; // jmarshall: should be fine, eval if broken, passed a function here?
				v7->SetNext(this->mFreeHead);
				destroyedParticle->Destroy();
				this->mFreeHead = v7;
			}
			else
			{
				v15 = v7;
				v7->Update(st->GetParticleTemplate(), time);				
			}
			v7 = next;
			if (!next)
				return;
			v8 = infinite;
		}
	}
}

bool rvSegment::UpdateParticles(rvBSE* effect, float time)
{
	rvSegmentTemplate* v4; // esi
	//connectdata* v6; // [esp+8h] [ebp-8h]
	int v7; // [esp+Ch] [ebp-4h]

	v4 = (rvSegmentTemplate * )mEffectDecl->GetSegmentTemplate(this->mSegmentTemplateHandle);
	if (!v4)
		return 0;
	Handle(effect, time);
	if ((v4->mFlags & 0x20) != 0
		|| v4->GetSmoker()
		|| (v4->mParticleTemplate.GetFlags() & 0x200) != 0
		|| v4->mParticleTemplate.GetNumTimeoutEffects())
	{
		UpdateGenericParticles(effect, v4, time);
	}
	else
	{
		UpdateSimpleParticles(time);
	}
// jmarshall - debug, uses undefined struct connectdata, since its debug, not worth dealing with.
	//if (bse_speeds.internalVar->integerValue)
	//{
	//	dword_11F4D54 += this->mActiveCount;
	//	if (this->mUsedHead)
	//		dword_11F4D58 += idFile::Tell(v6, v7);
	//}
// jmarshall end
	return this->mUsedHead != 0;
}

bool rvSegment::Active()
{
	rvSegmentTemplate* v1; // eax
	bool result; // al

	v1 = (rvSegmentTemplate * )mEffectDecl->GetSegmentTemplate(this->mSegmentTemplateHandle);
	if (v1 && (v1->mFlags & 4) != 0 && this->mActiveCount)
		result = v1->mFlags & 1;
	else
		result = 0;
	return result;
}

void rvSegment::AllocateSurface(rvBSE* effect, idRenderModel* model)
{
	rvSegmentTemplate* v4; // eax
	rvParticleTemplate* v5; // edi
	int v6; // ebp
	srfTriangles_t* v7; // eax
	int v9; // eax
	srfTriangles_t* v10; // eax
	idRenderModel* v11; // ebp

	v4 = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(this->mSegmentTemplateHandle);
	if (v4 && (v4->mFlags & 4) != 0)
	{
		v5 = &v4->mParticleTemplate;
		if (effect->GetLooping())
			v6 = this->mLoopParticleCount;
		else
			v6 = this->mParticleCount;
		v7 = model->AllocSurfaceTriangles(v6 * v4->mParticleTemplate.GetVertexCount(), v6 * v4->mParticleTemplate.GetIndexCount());

		//v7->texCoordScale = 100.0; // jmarshall: todo

		modelSurface_t surf;
		surf.id = 0;
		surf.geometry = v7;
		surf.shader = v5->GetMaterial();
		model->AddSurface(surf);


		mSurfaceIndex = model->NumSurfaces() - 1;
		if (v5->GetTrailType() == 2
			&& (v5->GetMaxTrailCount() || v5->GetMaxTrailTime() >= 0.0020000001))
		{
			v9 = v5->GetMaxTrailCount();
			v10 = model->AllocSurfaceTriangles(2 * v6 * v9 + 2, 12 * v6 * v9);

			modelSurface_t surf;
			surf.id = 0;
			surf.geometry = v10;
			surf.shader = v5->GetTrailMaterial();
			model->AddSurface(surf);

			this->mFlags |= 4u;
		}
	}
}

void rvSegment::ClearSurface(rvBSE* effect, idRenderModel* model)
{
	int v4; // ebp
	rvSegmentTemplate* v6; // eax
	rvParticleTemplate* v7; // ebp
	int v8; // eax
	int v9; // ebx
	int v10; // eax
	int v11; // ecx
	srfTriangles_t* v12; // eax
	modelSurface_t *v13; // edi
	int v14; // eax
	int v15; // ecx
	int v16; // eax
	srfTriangles_t* v17; // eax
	srfTriangles_t* v18; // eax
	srfTriangles_t* v19; // eax
	int v20; // [esp-8h] [ebp-10h]
	int v21; // [esp-4h] [ebp-Ch]
	int v22; // [esp-4h] [ebp-Ch]
	const modelSurface_t* v23; // [esp+18h] [ebp+10h]

	v6 = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(this->mSegmentTemplateHandle);
	if (v6 && (v6->mFlags & 4) != 0)
	{
		v21 = v4;
		v7 = &v6->mParticleTemplate;
		if (v6->mParticleTemplate.GetType() == 7)
		{
			v23 = model->Surface(this->mSurfaceIndex);
			model->FreeSurfaceTriangles(v23->geometry);
			if (effect->GetLooping())
				v8 = this->mLoopParticleCount;
			else
				v8 = this->mParticleCount;
			v9 = v8 * this->mActiveCount;
			v10 = v9 * v7->GetVertexCount();
			if (v10 > 10000)
				v10 = 10000;
			v11 = v9 * v7->GetIndexCount();
			if (v11 > 30000)
				v11 = 30000;
			v12 = model->AllocSurfaceTriangles(v10, v11);
			//model[2].__vftable = (idRenderModel_vtbl*)v12;
			//v12->texCoordScale = 100.0;
			if ((this->mFlags & 4) != 0)
			{
				v13 = (modelSurface_t * )model->Surface(this->mSurfaceIndex + 1);
				model->FreeSurfaceTriangles(v13->geometry);
				v14 = v9 * v7->GetMaxTrailCount();
				v15 = 2 * v14 + 2;
				if (v15 > 10000)
					v15 = 10000;
				v16 = 12 * v14;
				if (v16 > 30000)
					v16 = 30000;
				v17 = model->AllocSurfaceTriangles(v15, v16);
				v13->geometry = v17;
				//v17->texCoordScale = 100.0; // jmarshall
			}
		}
		else
		{
			v18 = model->Surface(mSurfaceIndex)->geometry;
			v18->numIndexes = 0;
			v18->numVerts = 0;
			if ((this->mFlags & 4) != 0)
			{
				v19 = model->Surface(mSurfaceIndex + 1)->geometry;
				v19->numIndexes = 0;
				v19->numVerts = 0;
			}
		}
	}
}

void rvSegment::RenderTrail(rvBSE* effect, const renderEffect_s* owner, idRenderModel* model, float time)
{
	rvSegmentTemplate* v6; // eax
	rvParticleTemplate* v7; // esi
	float v9; // [esp+10h] [ebp-4h]

	v6 = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(this->mSegmentTemplateHandle);
	if (v6)
	{
		v7 = &v6->mParticleTemplate;
		v9 = ceil(v6->mParticleTemplate.GetMaxTrailTime());
		if ((int)v9 != -1)
		{
			if (v7->GetMaxTrailTime() >= 0.0020000001 && v7->GetTrailType() == 2)
				rvSegment::RenderMotion(effect, owner, model, v7, time);
		}
	}
}

void rvSegment::Init(rvBSE* effect, const rvDeclEffect* effectDecl, int segmentTemplateHandle, float time)
{
	rvSegmentTemplate* v6; // edi

	this->mSegmentTemplateHandle = segmentTemplateHandle;
	this->mFlags = 0;
	this->mEffectDecl = effectDecl;
	this->mParticleType = effectDecl->GetSegmentTemplate(segmentTemplateHandle)->mParticleTemplate.GetType();
	this->mSurfaceIndex = -1;
	v6 = (rvSegmentTemplate * )effectDecl->GetSegmentTemplate(segmentTemplateHandle);
	if (v6)
	{
		this->mActiveCount = 0;
		this->mLastTime = time;
		this->mSecondsPerParticle.y = 0.0;
		this->mSecondsPerParticle.x = 0.0;
		this->mCount.x = 1.0;
		this->mCount.y = 1.0;
		this->mFreqShift = 1.0;
		this->mParticleCount = 0;
		this->mLoopParticleCount = 0;
		this->mParticles = 0;
		this->mSoundVolume = 0.0;
		InitTime(effect, v6, time);
		GetSecondsPerParticle(effect, v6, &v6->mParticleTemplate);
	}
}

void rvSegment::AddToParticleCount(rvBSE* effect, int count, int loopCount, float duration)
{
	rvSegmentTemplate* v6; // edi
	int v7; // eax
	int v8; // ebx
	const char* v9; // eax
	int v10; // ebx
	const char* v11; // eax
	int v12; // eax
	int v13; // eax
	char* X_4; // [esp+4h] [ebp-14h]
	char* X_4a; // [esp+4h] [ebp-14h]
	float durationa; // [esp+28h] [ebp+10h]
	float durationb; // [esp+28h] [ebp+10h]
	float durationc; // [esp+28h] [ebp+10h]

	v6 = (rvSegmentTemplate * )mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle);
	if (v6)
	{
		if (duration < (double)v6->mParticleTemplate.GetMaxDuration())
			duration = v6->mParticleTemplate.GetMaxDuration();
		durationa = duration + 0.01600000075995922;
		durationb = durationa / this->mSecondsPerParticle.y;
		durationc = ceil(durationb);
		v7 = (int)durationc;
		this->mLoopParticleCount += loopCount * (v7 + 1);
		this->mParticleCount += count * (v7 + 1);
		if (effect->GetLooping()) //((effect->mFlags & 1) != 0)
		{
			if (this->mLoopParticleCount > 2048)
			{
				common->Warning("More than MAX_PARTICLES required for segment %s\n", v6->GetSegmentName().c_str());				
				this->mLoopParticleCount = 2048;
			}
		}
		else if (this->mParticleCount > 2048)
		{
			common->Warning("More than MAX_PARTICLES required for segment %s\n", v6->GetSegmentName().c_str());
			this->mParticleCount = 2048;
		}
		v12 = count;
		if (count >= 2048)
			v12 = 2048;
		this->mParticleCount = v12;
		v13 = loopCount;
		if (loopCount >= 2048)
			v13 = 2048;
		this->mLoopParticleCount = v13;
	}
}

void rvSegment::CalcTrailCounts(rvBSE* effect, rvSegmentTemplate* st, rvParticleTemplate* pt, float duration)
{
	if (st->mTrailSegmentIndex >= 0) {
		effect->GetTrailSegment(st->mTrailSegmentIndex)->AddToParticleCount(effect, mParticleCount, mLoopParticleCount, duration);
	}
}

void rvSegment::CalcCounts(rvBSE* effect, float time)
{
	rvDeclEffect* effectDecl; // ebx
	int v4; // ebp
	bool v5; // zf
	rvSegmentTemplate* segmentTemplate; // ebp
	int segType; // eax
	int v8; // ecx
	int v9; // edi
	int v10; // esi
	int v11; // eax
	double v12; // st7
	bool v13; // cc
	char* v14; // edi
	int v15; // eax
	char* v16; // esi
	int v17; // eax
	int v18; // eax
	int v19; // eax
	float v21; // [esp+14h] [ebp-18h]
	float v22; // [esp+14h] [ebp-18h]
	float effectMinDuration; // [esp+1Ch] [ebp-10h]
	float pt; // [esp+20h] [ebp-Ch]
	float _X; // [esp+28h] [ebp-4h]
	float v27; // [esp+28h] [ebp-4h]
	float v28; // [esp+28h] [ebp-4h]
	float effecta; // [esp+30h] [ebp+4h]

	effectDecl = (rvDeclEffect * )this->mEffectDecl;
	v4 = this->mSegmentTemplateHandle;
	v5 = effectDecl->GetSegmentTemplate(v4) == 0;
	segmentTemplate = effectDecl->GetSegmentTemplate(v4);
	if (!v5)
	{
		segType = segmentTemplate->mSegType;
		if (segType != SEG_TRAIL)
		{
			v8 = segmentTemplate->mParticleTemplate.GetType();
			if (v8)
			{
				v21 = 0.0;
				v9 = 0;
				v10 = 0;
				effectMinDuration = segmentTemplate->GetParticleTemplate()->GetMaxDuration() + 0.01600000075995922;
				pt = effectDecl->GetMinDuration();
				switch (segType)
				{
				case SEG_EMITTER:
					if (v8 == 10)
					{
						v10 = 1;
						v9 = 1;
					}
					else
					{
						v22 = segmentTemplate->GetParticleTemplate()->GetMaxDuration() + 0.01600000075995922;
						if (segmentTemplate->mLocalDuration.y < (double)effectMinDuration)
							v22 = segmentTemplate->mLocalDuration.y;
						v21 = v22 + 0.01600000075995922;
						_X = v21 / this->mSecondsPerParticle.y;
						v11 = (int)ceilf(_X);
						v9 = v11 + 1;
						v10 = v11 + 1;
						if (effectMinDuration > (double)pt)
						{
							v27 = effectMinDuration * (double)(v11 + 1) / pt;
							v10 = (int)ceilf(v27) + 1;
						}
					}
					break;
				case SEG_SPAWNER:
					if (v8 == 10)
					{
						v10 = 1;
						v9 = 1;
					}
					else
					{
						v9 = (int)ceilf(this->mCount.y);
						v10 = v9;
						v12 = pt;
						if (pt != 0.0 && (segmentTemplate->mFlags & 0x20) == 0 && effectMinDuration > v12)
						{
							v28 = effectMinDuration / v12;
							v10 = v9 * ((int)ceilf(v28) + 1) + 1;
						}
					}
					break;
				case SEG_TRAIL:
					break;
				case SEG_DECAL:
				case SEG_LIGHT:
					v9 = 1;
					v10 = 1;
					break;
				default:
					v9 = 0;
					v10 = 0;
					break;
				}
				if (segmentTemplate->mSegType == 4)
					goto LABEL_37;
				if (effect->GetLooping())
				{
					if (v10 > 2048)
					{
						common->Warning("More than MAX_PARTICLES required for segment %s\n", segmentTemplate->GetSegmentName().c_str());					
						v10 = 2048;
					}
					v13 = v9 < 2048;
				}
				else
				{
					v13 = v9 < 2048;
					if (v9 > 2048)
					{
						common->Warning("More than MAX_PARTICLES required for segment %s\n", segmentTemplate->GetSegmentName().c_str());
						goto LABEL_27;
					}
				}
				if (v13)
				{
				LABEL_28:
					this->mParticleCount = v9;
					if (v10 >= 2048)
						v10 = 2048;
					this->mLoopParticleCount = v10;
					if ((segmentTemplate->mFlags & 4) != 0)
					{
						if (!v9 || !v10)
						{
							common->Warning("Segment with no particles for effect %s\n", effectDecl->base->GetName());							
						}
						v19 = segmentTemplate->mSegType;
						if (v19 == 2 || v19 == 3)
							CalcTrailCounts(effect, segmentTemplate, &segmentTemplate->mParticleTemplate, v21);
					}
				LABEL_37:
					if (!effect->GetLooping()) //effect->mFlags & 1) == 0)
					{
						effecta = this->mSegEndTime - time + segmentTemplate->mParticleTemplate.GetMaxDuration();
						effect->SetDuration(effecta);
					}
					return;
				}
			LABEL_27:
				v9 = 2048;
				goto LABEL_28;
			}
		}
	}
}

void rvSegment::ResetTime(rvBSE* effect, float time)
{
	rvSegmentTemplate* v3; // eax

	v3 = (rvSegmentTemplate*)mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle);
	if (v3)
	{
		if ((v3->mFlags & 0x20) == 0)
			InitTime(effect, v3, time);
	}
}

// jmarshall: this isn't like the original function, which was heavily optimized in the executable;
// basically here it allocates a block a rvParticle derieved classes, but its partially expecting a
// continous block even though its in a linked list. They probably had some optimizaiton here to
// prevent memory fragmentation. This needs to be looked at because 
rvParticle* rvSegment::InitParticleArray(rvBSE* effect) {
	int particleCount = 0;
	rvParticle* particle[2048]; // Max Particles

	if (effect->GetLooping())
		particleCount = this->mLoopParticleCount;
	else
		particleCount = this->mParticleCount;

	int type = mEffectDecl->GetSegmentTemplate(mSegmentTemplateHandle)->mParticleTemplate.GetType();

	for (int i = 0; i < particleCount - 1; i++)
	{
		rvParticle* newParticle = NULL;

		switch (type)
		{
			case PTYPE_LINE:
				newParticle = new rvLineParticle();
				break;

			case PTYPE_ORIENTED:
				newParticle = new rvOrientedParticle();
				break;

			case PTYPE_DECAL:
				newParticle = new rvDecalParticle();
				break;

			case PTYPE_MODEL:
				newParticle = new rvModelParticle();
				break;

			case PTYPE_LIGHT:
				newParticle = new rvLightParticle();
				break;

			case PTYPE_ELECTRICITY:
				newParticle = new rvElectricityParticle();
				break;

			case PTYPE_LINKED:
				newParticle = new rvLinkedParticle();
				break;

			case PTYPE_ORIENTEDLINKED:
				newParticle = new sdOrientedLinkedParticle();
				break;

			case PTYPE_DEBRIS:
				newParticle = new rvDebrisParticle();
				break;

			default:
				newParticle = new rvSpriteParticle();
				break;
		}

		particle[i] = newParticle;

		if (i > 0) {
			particle[i]->SetNext(newParticle);
		}
	}

	particle[particleCount - 1]->SetNext(NULL);

	mFreeHead = particle[0];
	mUsedHead = NULL;
	return particle[0];
}

void rvSegment::Sort(const idVec3& eyePos) {
	// This used smooth sort to sort the particles from the eye position.
	// Needs eval.
}

void rvSegment::InitParticles(rvBSE* effect)
{
	if (mEffectDecl->GetSegmentTemplate(this->mSegmentTemplateHandle))
	{
		mParticles = (rvParticle*)InitParticleArray(effect);
		mActiveCount = 0;
	}
}

void rvSegment::Render(rvBSE* effect, const renderEffect_s* owner, idRenderModel* model, float time)
{
// jmarshall
#if 0
	const rvDeclEffect* v5; // edx
	int v6; // eax
	bool v7; // zf
	rvSegmentTemplate* v8; // eax
	int v9; // ecx
	modelSurface_t* v10; // eax
	srfTriangles_t* v11; // edi
	int v12; // ecx
	//void(__thiscall * v13)(sdRenderSystemUtilities*, const idMat3*, const idVec3*, float*); // edx
	double v14; // st7
	rvSegment* v15; // esi
	double v16; // st6
	bool v17; // al
	void* v18; // esp
	int v19; // eax
	int v20; // eax
	rvParticle* v21; // ebx
	double v22; // st7
	rvParticleTemplate* v23; // esi
	//bool(__thiscall * v24)(rvParticle*, const rvBSE*, rvParticleTemplate*, const idMat3*, srfTriangles_t*, float, float); // edx
	bool v25; // sf
	int v26; // eax
	rvParticle* v27; // ebx
	int v28; // ecx
	int v29; // ecx
	float v30; // [esp+3Ch] [ebp-54h]
	int v31; // [esp+44h] [ebp-4Ch] BYREF
	float modelMatrix[16]; // [esp+50h] [ebp-40h] BYREF
	idMat3 view; // [esp+90h] [ebp+0h] BYREF
	float v34; // [esp+B4h] [ebp+24h]
	float v35; // [esp+B8h] [ebp+28h]
	float v36; // [esp+BCh] [ebp+2Ch]
	modelSurface_t* surf; // [esp+C0h] [ebp+30h]
	float v38; // [esp+C4h] [ebp+34h]
	int startIdx; // [esp+C8h] [ebp+38h]
	rvParticleTemplate* pt; // [esp+CCh] [ebp+3Ch]
	rvSegmentTemplate* st; // [esp+D0h] [ebp+40h]
	int startVtx; // [esp+D4h] [ebp+44h]
	float v43; // [esp+D8h] [ebp+48h]
	float v44; // [esp+DCh] [ebp+4Ch]
	float v45; // [esp+E0h] [ebp+50h]
	rvSegment* v46; // [esp+E4h] [ebp+54h]
	int numAllocedIndices; // [esp+E8h] [ebp+58h]
	int numAllocedVerts; // [esp+ECh] [ebp+5Ch]
	rvParticle** inverseList; // [esp+F0h] [ebp+60h]
	int numRender; // [esp+F4h] [ebp+64h]
	int i; // [esp+104h] [ebp+74h]
	int ia; // [esp+104h] [ebp+74h]
	float modela; // [esp+108h] [ebp+78h]
	bool model_3; // [esp+10Bh] [ebp+7Bh]

	v5 = this->mEffectDecl;
	v6 = this->mSegmentTemplateHandle;
	v7 = v5->GetSegmentTemplate(v6) == 0;
	v8 = (rvSegmentTemplate * )v5->GetSegmentTemplate(v6);
	v46 = this;
	st = v8;
	if (!v7)
	{
		v9 = this->mSurfaceIndex;
		pt = &v8->mParticleTemplate;
		v10 = (modelSurface_t *)model->Surface(v9);
		v11 = v10->geometry;
		v12 = v11->numIndexes;
		surf = v10;
		startVtx = v11->numVerts;
		startIdx = v12;
		numRender = 0;
		R_AxisToModelMatrix(owner->axis, owner->origin, modelMatrix);
		R_GlobalVectorToLocal(modelMatrix, effect->GetViewAxis()[1], view[1]);
		R_GlobalVectorToLocal(modelMatrix, effect->GetViewAxis()[2], view[2]);
		v43 = effect->GetViewOrg().x - owner->origin.x;
		v44 = effect->GetViewOrg().y - owner->origin.y;
		v45 = effect->GetViewOrg().z - owner->origin.z;
		v35 = owner->axis[2].z;
		v34 = owner->axis[1].z;
		//numAllocedIndices = SLODWORD(owner->axis.mat[0].z); <-- nonsense optimized garbage.
		v36 = owner->axis[2].y;
		//numAllocedVerts = SLODWORD(owner->axis.mat[1].y); <-- nonsense optimized garbage.
		modela = owner->axis[0].y;
		v38 = owner->axis[2].x;
		//inverseList = (rvParticle**)LODWORD(owner->axis.mat[1].x); <-- nonsense optimized garbage.
		v14 = v44;
		v15 = v46;
		v16 = v43;
		v43 = owner->axis[0].x * v43 + v44 * modela + v45 * owner->axis[0].z; // (SLODWORD(owner->axis.mat[0].z))
		v44 = v44 * owner->axis[1].y + owner->axis[1].x * v16 + v45 * v34;
		v45 = v45 * v35 + v16 * v38 + v14 * v36;
		view[0].x = v43;
		view[0].y = v44;
		view[0].z = v45;
// jmarshall - todo sort particles.		
		//if (r_sortParticles.internalVar->integerValue
		//	&& (v46->mEffectDecl->mSegmentTemplates.list[v46->mSegmentTemplateHandle].mFlags & 0x400) != 0)
		//{
		//	rvSegment::Sort(v46, view.mat);
		//}
// jmarshall end

// jmarshall - inverse draw order
		//v17 = st->GetInverseDrawOrder();//(st->mFlags & 0x800) != 0;
		//v7 = !st->GetInverseDrawOrder(); //(st->mFlags & 0x800) == 0;
		v17 = false;
		v7 = true;
// jmarshall end

		//*(float*)&inverseList = 0.0; <-- looks like compiler generated nonsense from above. 
		i = 0;
		model_3 = v17;
// jmarshall - not sure what to do here, compiler nuked this statement pretty badly.
// inverse draw order.
		//if (!v7)
		//{
		//	v18 = alloca(4 * v15->mActiveCount);
		//	*(float*)&inverseList = COERCE_FLOAT(&v31);
		//}
// jmarshall end

		v19 = v11->numVerts;
		numAllocedVerts = 9500;
		if (v19 <= 9500)
			numAllocedVerts = v19;
		v20 = v11->numIndexes;
		numAllocedIndices = 29500;
		if (v20 <= 29500)
			numAllocedIndices = v20;
		v21 = v15->mUsedHead;
		v22 = time;
		if (v21)
		{
			while (1)
			{
				if ((st->mFlags & 0x20) != 0)
					v21->mEndTime = v22 + 1.0;
				v23 = pt;
				if (v11->numVerts + pt->GetVertexCount() > numAllocedVerts || v11->numIndexes + pt->GetIndexCount() > numAllocedIndices)
					break;
				if (model_3)
				{
					inverseList[i++] = v21;
				}
				else
				{
					++numRender;
					v30 = v22;
					if (v21->Render(effect, pt, view, v11, v30, 1.0f) && v23->GetTrailType() == 1)
					{
						v21->RenderBurnTrail(effect, v23, view, v11, time);						
					}
				}
				v21 = v21->mNext;
				if (!v21)
				{
					v23 = pt;
					break;
				}
				v22 = time;
			}
		}
		else
		{
			v23 = pt;
		}
// jmarshall - inverse.
		//if (model_3)
		//{
		//	v25 = i - 1 < 0;
		//	v26 = i - 1;
		//	ia = i - 1;
		//	if (!v25)
		//	{
		//		numRender += v26 + 1;
		//		while (1)
		//		{
		//			v27 = inverseList[v26];
		//			if (((unsigned __int8(__thiscall*)(rvParticle*, rvBSE*, rvParticleTemplate*, idMat3*, srfTriangles_t*, _DWORD, _DWORD))v27->Render)(
		//				v27,
		//				effect,
		//				v23,
		//				&view,
		//				v11,
		//				LODWORD(time),
		//				1.0)
		//				&& v23->mTrailInfo->mTrailType == 1)
		//			{
		//				((void(__thiscall*)(rvParticle*, rvBSE*, rvParticleTemplate*, idMat3*, srfTriangles_t*, _DWORD))v27->RenderBurnTrail)(
		//					v27,
		//					effect,
		//					v23,
		//					&view,
		//					v11,
		//					LODWORD(time));
		//			}
		//			if (--ia < 0)
		//				break;
		//			v26 = ia;
		//		}
		//	}
		//}
// jmarshall end
		v28 = v46->mActiveCount;
		if (v11->numVerts > v28 * v23->GetVertexCount())
			common->Printf("rvSegment::Render - tri->numVerts > pt->GetVertexCount() * mActiveCount ( [%d %d] [%d %d] [%d %d] [%d %d %d] )",
				startVtx,
				startIdx,
				v11->numVerts,
				v11->numIndexes,
				numRender,
				v28,
				v23->GetIndexCount(),
				v23->GetVertexCount(),
				model_3);
		v29 = v46->mActiveCount;
		if (v11->numIndexes > v29 * v23->GetIndexCount())
			common->Printf("rvSegment::Render - tri->numIndexes > pt->GetIndexCount() * mActiveCount ( [%d %d] [%d %d] [%d %d] [%d %d %d] )",
				startVtx,
				startIdx,
				v11->numVerts,
				v11->numIndexes,
				numRender,
				v29,
				v23->GetIndexCount(),
				v23->GetVertexCount(),
				model_3);
		R_BoundTriSurf(surf->geometry);
	}
#endif
// jmarshall end
}

void rvSegment::CreateDecal(rvBSE* effect, float time)
{
	// jmarshall: todo
}
#endif
