export function Name() { return "AngryMiao Neon 80"; }
export function Version() { return "1.1.0"; }
export function VendorId() { return 0xa8f8; }
export function ProductId() { return 0x4e80; }
export function Publisher() { return "AngryMiao"; }
export function Documentation() { return "qmk/am-neon-80-signalrgb"; }
export function DeviceType() { return "keyboard"; }
export function Size() { return [46, 19]; }
export function DefaultPosition() { return [10, 100]; }
export function DefaultScale() { return 8.0; }

/* global
shutdownMode:readonly
shutdownColor:readonly
LightingMode:readonly
forcedColor:readonly
*/

export function ControllableParameters() {
	return [
		{"property":"shutdownMode", "group":"lighting", "label":"Shutdown Mode", "type":"combobox", "values":["SignalRGB", "Hardware"], "default":"SignalRGB"},
		{"property":"shutdownColor", "group":"lighting", "label":"Shutdown Color", "min":"0", "max":"360", "type":"color", "default":"#000000"},
		{"property":"LightingMode", "group":"lighting", "label":"Lighting Mode", "type":"combobox", "values":["Canvas", "Forced"], "default":"Canvas"},
		{"property":"forcedColor", "group":"lighting", "label":"Forced Color", "min":"0", "max":"360", "type":"color", "default":"#00a19a"},
	];
}

const COMMAND_INFO = 0x60;
const COMMAND_MODE = 0x61;
const COMMAND_STREAM = 0x62;
const COMMAND_COMMIT_FRAME = 0x63;

const ZONE_KEY = 0;
const ZONE_HEAD = 1;
const ZONE_SIDE = 2;

const MAX_LEDS_PER_PACKET = 14;
const RGB565_BYTES_PER_LED = 2;

const KEY_LED_COUNT = 89;
const HEAD_LED_COUNT = 46 * 5;
const SIDE_LED_COUNT = 70;
const TOTAL_LED_COUNT = KEY_LED_COUNT + HEAD_LED_COUNT + SIDE_LED_COUNT;
const KEY_X_OFFSET = 1.8;
const KEY_Y_OFFSET = 10.0;
const KEY_X_SCALE = 2.35;
const KEY_Y_SCALE = 1.35;
const HEAD_X_OFFSET = 0;
const HEAD_Y_OFFSET = 5.4;
const HEAD_Y_STEP = 0.95;

const VIAL_KEYMAP = [
	[
		{"c": "#777777"},
		"0,0",
		{"x": 0.25, "c": "#cccccc"},
		"0,1", "0,2", "0,3", "0,4",
		{"x": 0.25},
		"0,5", "0,6", "0,7", "0,8",
		{"x": 0.25},
		"0,9", "0,10", "0,11", "0,12",
		{"x": 0.25, "c": "#aaaaaa"},
		"0,13",
		{"x": 0.25, "c": "#777777"},
		"0,14", "3,14", "3,13",
	],
	[
		{"y": 0.25, "c": "#aaaaaa"},
		"1,0", "1,1", "1,2", "1,3", "1,4", "1,5", "1,6", "1,7", "1,8", "1,9", "1,10", "1,11", "1,12",
		{"w": 2},
		"1,13",
		{"x": 0.25, "c": "#777777"},
		"1,14", "4,14", "4,13",
	],
	[
		{"c": "#aaaaaa", "w": 1.5},
		"2,0", "2,1", "2,2", "2,3", "2,4", "2,5", "2,6", "2,7", "2,8", "2,9", "2,10", "2,11", "2,12",
		{"w": 1.5},
		"2,13",
		{"x": 0.25, "c": "#777777"},
		"2,14", "5,14", "4,12",
	],
	[
		{"c": "#aaaaaa", "w": 1.75},
		"3,0", "3,1", "3,2", "3,3", "3,4", "3,5", "3,6", "3,7", "3,8", "3,9", "3,10", "3,11",
		{"w": 2.25},
		"3,12",
	],
	[
		{"c": "#aaaaaa", "w": 2.25},
		"4,0", "4,1", "4,2", "4,3", "4,4", "4,5", "4,6", "4,7", "4,8", "4,9", "4,10",
		{"w": 2.75},
		"4,11",
		{"x": 1.25},
		"5,12",
	],
	[
		{"c": "#aaaaaa", "w": 1.5},
		"5,0", "5,1",
		{"w": 1.5},
		"5,2",
		{"w": 7},
		"5,3",
		{"w": 1.5},
		"5,10", "5,11",
		{"w": 1.5},
		"5,13",
		{"x": 0.25},
		"5,8", "5,7", "5,6",
	],
];

const KEY_LED_SEQUENCE = [
	{name: "Esc", matrix: "0,0"},
	{name: "F1", matrix: "0,1"},
	{name: "F2", matrix: "0,2"},
	{name: "F3", matrix: "0,3"},
	{name: "F4", matrix: "0,4"},
	{name: "F5", matrix: "0,5"},
	{name: "F6", matrix: "0,6"},
	{name: "F7", matrix: "0,7"},
	{name: "F8", matrix: "0,8"},
	{name: "F9", matrix: "0,9"},
	{name: "F10", matrix: "0,10"},
	{name: "F11", matrix: "0,11"},
	{name: "F12", matrix: "0,12"},
	{name: "Mute", matrix: "0,13"},
	{name: "Print Screen", matrix: "0,14"},
	{name: "Scroll Lock", matrix: "3,14"},
	{name: "Pause", matrix: "3,13"},

	{name: "`", matrix: "1,0"},
	{name: "1", matrix: "1,1"},
	{name: "2", matrix: "1,2"},
	{name: "3", matrix: "1,3"},
	{name: "4", matrix: "1,4"},
	{name: "5", matrix: "1,5"},
	{name: "6", matrix: "1,6"},
	{name: "7", matrix: "1,7"},
	{name: "8", matrix: "1,8"},
	{name: "9", matrix: "1,9"},
	{name: "0", matrix: "1,10"},
	{name: "-", matrix: "1,11"},
	{name: "=", matrix: "1,12"},
	{name: "Backspace", matrix: "1,13"},
	{name: "Insert", matrix: "1,14"},
	{name: "Home", matrix: "4,14"},
	{name: "Page Up", matrix: "4,13"},

	{name: "Tab", matrix: "2,0"},
	{name: "Q", matrix: "2,1"},
	{name: "W", matrix: "2,2"},
	{name: "E", matrix: "2,3"},
	{name: "R", matrix: "2,4"},
	{name: "T", matrix: "2,5"},
	{name: "Y", matrix: "2,6"},
	{name: "U", matrix: "2,7"},
	{name: "I", matrix: "2,8"},
	{name: "O", matrix: "2,9"},
	{name: "P", matrix: "2,10"},
	{name: "[", matrix: "2,11"},
	{name: "]", matrix: "2,12"},
	{name: "\\", matrix: "2,13"},
	{name: "Delete", matrix: "2,14"},
	{name: "End", matrix: "5,14"},
	{name: "Page Down", matrix: "4,12"},

	{name: "Caps Lock", matrix: "3,0"},
	{name: "A", matrix: "3,1"},
	{name: "S", matrix: "3,2"},
	{name: "D", matrix: "3,3"},
	{name: "F", matrix: "3,4"},
	{name: "G", matrix: "3,5"},
	{name: "H", matrix: "3,6"},
	{name: "J", matrix: "3,7"},
	{name: "K", matrix: "3,8"},
	{name: "L", matrix: "3,9"},
	{name: ";", matrix: "3,10"},
	{name: "'", matrix: "3,11"},
	{name: "Enter", matrix: "3,12"},

	{name: "Left Shift", matrix: "4,0"},
	{name: "Z", matrix: "4,1"},
	{name: "X", matrix: "4,2"},
	{name: "C", matrix: "4,3"},
	{name: "V", matrix: "4,4"},
	{name: "B", matrix: "4,5"},
	{name: "N", matrix: "4,6"},
	{name: "M", matrix: "4,7"},
	{name: ",", matrix: "4,8"},
	{name: ".", matrix: "4,9"},
	{name: "/", matrix: "4,10"},
	{name: "Right Shift", matrix: "4,11"},
	{name: "Up Arrow", matrix: "5,12"},

	{name: "Left Ctrl", matrix: "5,0"},
	{name: "Left Win", matrix: "5,1"},
	{name: "Left Alt", matrix: "5,2"},
	{name: "Space Left", matrix: "5,3", fraction: 0.25},
	{name: "Space Center", matrix: "5,3", fraction: 0.5},
	{name: "Space Right", matrix: "5,3", fraction: 0.75},
	{name: "Right Alt", matrix: "5,10"},
	{name: "Right Win", matrix: "5,11"},
	{name: "Right Ctrl", matrix: "5,13"},
	{name: "Left Arrow", matrix: "5,8"},
	{name: "Down Arrow", matrix: "5,7"},
	{name: "Right Arrow", matrix: "5,6"},
];

const KEY_LED_POSITIONS = [
	[1, 10.675], 	[5, 10.675], 	[7, 10.675], 	[9, 10.675], 		[11, 10.675],	[15, 10.675], 	[17, 10.675], 	[19, 10.675], [21, 10.675], [26, 10.675],[28, 10.675], [30, 10.675], [32, 10.675], [36, 10.675], [40, 10.675],[42, 10.675], [44, 10.675],
	[1, 12.3625], 	[4, 12.3625], 	[6, 12.3625], 	[8.025, 12.3625], 	[10, 12.3625],	[14, 12.3625], 	[16, 12.3625], 	[18, 12.3625],[20, 12.3625], [24, 12.3625],[26, 12.3625], [28, 12.3625], [30, 12.3625], [34, 12.3625], [40, 12.3625],[42, 12.3625], [44, 12.3625],
	[2, 13.375], 	[5, 13.375], 	[7, 13.375], 	[9, 13.375], 		[11, 13.375],	[15, 13.375], 	[17, 13.375], 	[19, 13.375], [21, 13.375], [25, 13.375],[27, 13.375], [29, 13.375], [31, 13.375], [35.2875, 13.375], [40, 13.375],[42.1625, 13.375], [44, 13.375],
	[2, 14], 		[5, 14], 		[7, 14], 		[9, 14], 			[11, 14],		[15, 14], 		[17, 14], 		[19, 14],	  [21, 14], [26, 14],		[28, 14], [30, 14], [35, 14],
	[3, 15], 						[6, 15], 		[8, 15], [10, 15], [15.3125, 15],[17.6625, 15], [20.0125, 15], [22.3625, 15], [24.7125, 15], [27.0625, 15],[29.4125, 15], [33.81875, 15], [42, 15],
	[2, 16], 		[5, 16], 		[7, 16], 		[10, 16], [19.425, 16],[27, 16], [29.4125, 16], [32.35, 16], [35.2875, 16], [40, 16],[42, 16], [44, 16],
];

const SIDE_LED_POSITIONS = [
	[18, 0.5], [19, 0.5], [20, 0.5], [21, 0.5], [22, 0.5], [23, 0.5], [24, 0.5], [25, 0.5], [26, 0.5], [27, 0.5],
	[13.5, 1.6], [14.5, 1.6], [15.5, 1.6], [16.5, 1.6], [17.5, 1.6], [18.5, 1.6], [19.5, 1.6], [20.5, 1.6], [21.5, 1.6], [22.5, 1.6],
	[23.5, 1.6], [24.5, 1.6], [25.5, 1.6], [26.5, 1.6], [27.5, 1.6], [28.5, 1.6], [29.5, 1.6], [30.5, 1.6], [31.5, 1.6],
	[12.5, 2.7], [13.5, 2.7], [14.5, 2.7], [15.5, 2.7], [16.5, 2.7], [17.5, 2.7], [18.5, 2.7], [19.5, 2.7], [20.5, 2.7], [21.5, 2.7],
	[22.5, 2.7], [23.5, 2.7], [24.5, 2.7], [25.5, 2.7], [26.5, 2.7], [27.5, 2.7], [28.5, 2.7], [29.5, 2.7], [30.5, 2.7], [31.5, 2.7],
	[32.5, 2.7],
	[13, 3.8], [14, 3.8], [15, 3.8], [16, 3.8], [17, 3.8], [18, 3.8], [19, 3.8], [20, 3.8], [21, 3.8], [22, 3.8],
	[23, 3.8], [24, 3.8], [25, 3.8], [26, 3.8], [27, 3.8], [28, 3.8], [29, 3.8], [30, 3.8], [31, 3.8], [32, 3.8],
];

const LED_NAMES = [];
const LED_POSITIONS = [];
const MATRIX_LAYOUT = buildMatrixLayout();
const ZONE_PACKET_STRIDE = MAX_LEDS_PER_PACKET * RGB565_BYTES_PER_LED;

let infoReadSucceeded = false;
let forceFullFrameSend = true;
let previousZoneFrames = {
	[ZONE_KEY]: [],
	[ZONE_HEAD]: [],
	[ZONE_SIDE]: [],
};

buildLayout();

export function LedNames() {
	return LED_NAMES;
}

export function LedPositions() {
	return LED_POSITIONS;
}

export function Initialize() {
	requestInfo();
	setMode(true);
	resetFrameCache();
}

export function Render() {
	sendAllColors();
}

export function Shutdown(systemSuspending) {
	if (systemSuspending) {
		sendAllColors("#000000");
		return;
	}

	if (shutdownMode === "SignalRGB") {
		sendAllColors(shutdownColor);
		return;
	}

	setMode(false);
}

export function Validate(endpoint) {
	return endpoint.interface === 1;
}

export function Image() {
	return "";
}

function buildLayout() {
	buildUnderKeyLayout();
	buildHeadLayout();
	buildSideLayout();
}

function buildUnderKeyLayout() {
	for (let index = 0; index < KEY_LED_SEQUENCE.length; index++) {
		const key = KEY_LED_SEQUENCE[index];
		LED_NAMES.push(`Key - ${key.name}`);
		LED_POSITIONS.push(KEY_LED_POSITIONS[index] || [0, 0]);
	}
}

function buildHeadLayout() {
	for (let y = 0; y < 5; y++) {
		for (let x = 0; x < 46; x++) {
			LED_NAMES.push(`Head Matrix ${y + 1}-${x + 1}`);
			LED_POSITIONS.push([HEAD_X_OFFSET + x, HEAD_Y_OFFSET + (y * HEAD_Y_STEP)]);
		}
	}
}

function buildSideLayout() {
	for (let index = 0; index < SIDE_LED_POSITIONS.length; index++) {
		LED_NAMES.push(`Front Matrix ${index + 1}`);
		LED_POSITIONS.push(SIDE_LED_POSITIONS[index] || [0, 0]);
	}
}

function buildMatrixLayout() {
	const matrixLayout = new Map();
	let currentY = 0;

	for (const row of VIAL_KEYMAP) {
		let x = 0;
		let y = currentY;
		let width = 1;
		let height = 1;

		for (const item of row) {
			if (typeof item === "string") {
				matrixLayout.set(item, {x, y, w: width, h: height});
				x += width;
				width = 1;
				height = 1;
				continue;
			}

			if (typeof item.y === "number") {
				y = currentY + item.y;
			}
			if (typeof item.x === "number") {
				x += item.x;
			}
			if (typeof item.w === "number") {
				width = item.w;
			}
			if (typeof item.h === "number") {
				height = item.h;
			}
		}

		currentY += 1;
	}

	return matrixLayout;
}

function getMatrixLedPosition(matrix, fraction) {
	const key = MATRIX_LAYOUT.get(matrix);
	if (!key) {
		return [0, 0];
	}

	const centerX = typeof fraction === "number" ? (key.x + (key.w * fraction)) : (key.x + (key.w / 2));
	const centerY = key.y + (key.h / 2);

	return [
		KEY_X_OFFSET + (centerX * KEY_X_SCALE),
		KEY_Y_OFFSET + (centerY * KEY_Y_SCALE),
	];
}

function requestInfo() {
	device.write([0x00, COMMAND_INFO], 33);
	device.pause(20);

	const response = device.read([0x00], 32, 20);
	if (device.getLastReadSize() <= 0) {
		device.notify("SignalRGB Firmware Missing", "Neon 80 did not respond to the custom SignalRGB handshake.", 2, "Documentation");
		return;
	}

	if (response[1] !== COMMAND_INFO) {
		device.notify("SignalRGB Handshake Failed", "Neon 80 returned an unexpected handshake packet.", 2, "Documentation");
		return;
	}

	infoReadSucceeded = true;
	device.log(`Neon 80 protocol ${response[2]}.${response[3]}.${response[4]}`);
	device.log(`Neon 80 UID ${String.fromCharCode(response[5], response[6], response[7])}`);
	device.log(`Neon 80 zones key=${response[8]} head=${response[9]} side=${response[10]}`);
}

function setMode(enabled) {
	device.write([0x00, COMMAND_MODE, enabled ? 1 : 0], 33);
	device.pause(5);
	forceFullFrameSend = enabled;
}

function createSolidColorArray(color) {
	const rgbData = new Array(TOTAL_LED_COUNT * 3).fill(0);
	for (let ledIndex = 0; ledIndex < TOTAL_LED_COUNT; ledIndex++) {
		const offset = ledIndex * 3;
		rgbData[offset + 0] = color[0];
		rgbData[offset + 1] = color[1];
		rgbData[offset + 2] = color[2];
	}
	return rgbData;
}

function grabColors(overrideColor) {
	if (overrideColor) {
		return createSolidColorArray(hexToRgb(overrideColor));
	}

	if (LightingMode === "Forced") {
		return createSolidColorArray(hexToRgb(forcedColor));
	}

	const rgbData = new Array(TOTAL_LED_COUNT * 3).fill(0);
	for (let ledIndex = 0; ledIndex < TOTAL_LED_COUNT; ledIndex++) {
		const position = LED_POSITIONS[ledIndex];
		const color = device.color(position[0], position[1]);
		const offset = ledIndex * 3;
		rgbData[offset + 0] = color[0];
		rgbData[offset + 1] = color[1];
		rgbData[offset + 2] = color[2];
	}
	return rgbData;
}

function sendAllColors(overrideColor) {
	if (!infoReadSucceeded) {
		requestInfo();
	}

	const rgbData = grabColors(overrideColor);
	sendZoneColors(ZONE_KEY, rgbData.slice(0, KEY_LED_COUNT * 3));
	sendZoneColors(ZONE_HEAD, rgbData.slice(KEY_LED_COUNT * 3, (KEY_LED_COUNT + HEAD_LED_COUNT) * 3));
	sendZoneColors(ZONE_SIDE, rgbData.slice((KEY_LED_COUNT + HEAD_LED_COUNT) * 3));
	commitFrame();
	forceFullFrameSend = false;
}

function sendZoneColors(zone, rgbData) {
	const packedZoneData = packRgb888ArrayTo565(rgbData);
	const previousFrame = previousZoneFrames[zone];

	for (let byteOffset = 0; byteOffset < packedZoneData.length; byteOffset += ZONE_PACKET_STRIDE) {
		const bytesToSend = Math.min(ZONE_PACKET_STRIDE, packedZoneData.length - byteOffset);
		const ledOffset = Math.floor(byteOffset / RGB565_BYTES_PER_LED);
		const ledCount = Math.floor(bytesToSend / RGB565_BYTES_PER_LED);

		if (!forceFullFrameSend && chunkMatches(previousFrame, packedZoneData, byteOffset, bytesToSend)) {
			continue;
		}

		const chunk = packedZoneData.slice(byteOffset, byteOffset + bytesToSend);
		const packet = [0x00, COMMAND_STREAM, zone, ledOffset, ledCount].concat(chunk);
		device.write(packet, 33);
	}

	previousZoneFrames[zone] = packedZoneData.slice();
}

function packRgb888ArrayTo565(rgbData) {
	const packed = [];

	for (let offset = 0; offset < rgbData.length; offset += 3) {
		const red = rgbData[offset + 0] >> 3;
		const green = rgbData[offset + 1] >> 2;
		const blue = rgbData[offset + 2] >> 3;
		const rgb565 = (red << 11) | (green << 5) | blue;

		packed.push((rgb565 >> 8) & 0xFF, rgb565 & 0xFF);
	}

	return packed;
}

function chunkMatches(previousFrame, currentFrame, offset, length) {
	if (!previousFrame || previousFrame.length !== currentFrame.length) {
		return false;
	}

	for (let index = 0; index < length; index++) {
		if (previousFrame[offset + index] !== currentFrame[offset + index]) {
			return false;
		}
	}

	return true;
}

function resetFrameCache() {
	previousZoneFrames = {
		[ZONE_KEY]: [],
		[ZONE_HEAD]: [],
		[ZONE_SIDE]: [],
	};
	forceFullFrameSend = true;
}

function commitFrame() {
	device.write([0x00, COMMAND_COMMIT_FRAME], 33);
}

function hexToRgb(hex) {
	const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
	if (!result) {
		return [0, 0, 0];
	}

	return [
		parseInt(result[1], 16),
		parseInt(result[2], 16),
		parseInt(result[3], 16),
	];
}
