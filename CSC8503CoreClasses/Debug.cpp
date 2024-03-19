#include "Debug.h"
using namespace NCL;

std::vector<Debug::DebugStringEntry>	Debug::stringEntries;
std::vector<Debug::DebugLineEntry>		Debug::lineEntries;

SimpleFont* Debug::debugFont = nullptr;

const Vector4 Debug::RED		= Vector4(1, 0, 0, 1);
const Vector4 Debug::GREEN		= Vector4(0, 1, 0, 1);
const Vector4 Debug::BLUE		= Vector4(0, 0, 1, 1);

const Vector4 Debug::BLACK		= Vector4(0, 0, 0, 1);
const Vector4 Debug::WHITE		= Vector4(1, 1, 1, 1);

const Vector4 Debug::YELLOW		= Vector4(1, 1, 0, 1);
const Vector4 Debug::MAGENTA	= Vector4(1, 0, 1, 1);
const Vector4 Debug::CYAN		= Vector4(0, 1, 1, 1);

void Debug::Print(const std::string& text, const Vector2& pos, const Vector4& colour, float fontSize) {
	DebugStringEntry newEntry;

	newEntry.data = text;
	newEntry.position = pos;
	newEntry.colour = colour;
	newEntry.fontSize = fontSize;

	stringEntries.emplace_back(newEntry);
}

void Debug::DrawLine(const Vector3& startpoint, const Vector3& endpoint, const Vector4& colour, float time) {
	DebugLineEntry newEntry;

	newEntry.start = startpoint;
	newEntry.end = endpoint;
	newEntry.colourA = colour;
	newEntry.colourB = colour;
	newEntry.time = time;

	lineEntries.emplace_back(newEntry);
}

void Debug::DrawAxisLines(const Matrix4& modelMatrix, float scaleBoost, float time) {
	Matrix4 local = modelMatrix;
	local.SetPositionVector({ 0, 0, 0 });

	Vector3 fwd = local * Vector4(0, 0, -1, 1.0f);
	Vector3 up = local * Vector4(0, 1, 0, 1.0f);
	Vector3 right = local * Vector4(1, 0, 0, 1.0f);

	Vector3 worldPos = modelMatrix.GetPositionVector();

	DrawLine(worldPos, worldPos + (right * scaleBoost), Debug::RED, time);
	DrawLine(worldPos, worldPos + (up * scaleBoost), Debug::GREEN, time);
	DrawLine(worldPos, worldPos + (fwd * scaleBoost), Debug::BLUE, time);
}

void Debug::DrawCube(const Vector3& halfDimensions, const Vector3& position) {
	Vector3 vertices[4] = { {1, 1, 1}, {1, -1, -1}, {-1, 1, -1}, {-1, -1, 1} };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			Vector3 vertex = vertices[i];
			vertex[j] *= -1;
			DrawLine(position + (vertices[i] * halfDimensions), position + (vertex * halfDimensions), Debug::GREEN);
		}
	}
}

void Debug::DrawRotatedCube(const Vector3& halfDimensions, const Vector3& position, const Quaternion& orientation) {
	Vector3 vertices[4] = { {1, 1, 1}, {1, -1, -1}, {-1, 1, -1}, {-1, -1, 1} };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			Vector3 vertex = vertices[i];
			vertex[j] *= -1;
			DrawLine(position + (orientation * (vertices[i] * halfDimensions)), position + (orientation * (vertex * halfDimensions)), Debug::GREEN);
		}
	}
}

void Debug::DrawSphere(float radius, const Vector3& position) {
	const int circleAmount = 32;
	Vector3 spin[3] = { Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0) };
	Vector3 translations[3] = { Vector3(0, radius, 0), Vector3(0, radius, 0), Vector3(radius, 0, 0) };
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < circleAmount; j++) {
			Vector3 start = position + (Matrix4::Rotation(360 / (circleAmount - 2) * j, spin[i]) * Matrix4::Translation(translations[i])).GetPositionVector();
			Vector3 end = position + (Matrix4::Rotation(360 / (circleAmount - 2) * (j + 1), spin[i]) * Matrix4::Translation(translations[i])).GetPositionVector();
			DrawLine(start, end, Debug::GREEN);
		}
	}
}

void Debug::UpdateRenderables(float dt) {
	int trim = 0;
	for (int i = 0; i < lineEntries.size(); ) {
		DebugLineEntry* e = &lineEntries[i];
		e->time -= dt;
		if (e->time < 0) {
			trim++;
			lineEntries[i] = lineEntries[lineEntries.size() - trim];
		}
		else {
			++i;
		}
		if (i + trim >= lineEntries.size()) {
			break;
		}
	}
	lineEntries.resize(lineEntries.size() - trim);
	stringEntries.clear();
}

SimpleFont* Debug::GetDebugFont() {
	return debugFont;
}

void Debug::CreateDebugFont(const std::string& dataFile, Texture& tex) {
	debugFont = new SimpleFont(dataFile, tex);
}

const std::vector<Debug::DebugStringEntry>& Debug::GetDebugStrings() {
	return stringEntries;
}

const std::vector<Debug::DebugLineEntry>& Debug::GetDebugLines() {
	return lineEntries;
}

void Debug::ClearStringEntries() {
	stringEntries.clear();
}
