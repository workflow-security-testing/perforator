/* eslint-disable no-bitwise */
import type { Coordinate, H, I } from './renderer';

/**
 * Densely packed coordinates for memory-efficient storage
 * Uses a flat array where:
 * - Index 2n contains H (height/level)
 * - Index 2n+1 contains I (index within level)
 *
 * Example: [h0, i0, h1, i1, h2, i2, ...]
 */

export type DenselyPackedCoordinates = number[];

export function getDenseLength(coords: DenselyPackedCoordinates): number {
    return coords.length >>> 1; // equivalent to Math.floor(coords.length / 2)
}

export function getDenseH(coords: DenselyPackedCoordinates, n: number): H {
    return coords[n << 1]; // equivalent to coords[n * 2]
}

export function getDenseI(coords: DenselyPackedCoordinates, n: number): I {
    return coords[(n << 1) + 1]; // equivalent to coords[n * 2 + 1]
}

export function pushDenseCoord(coords: DenselyPackedCoordinates, h: H, i: I): void {
    coords.push(h, i);
}

export function toDenseCoordinates(coords: Coordinate[]): DenselyPackedCoordinates {
    const result: DenselyPackedCoordinates = new Array(coords.length * 2);
    for (let n = 0; n < coords.length; n++) {
        result[n << 1] = coords[n][0];
        result[(n << 1) + 1] = coords[n][1];
    }
    return result;
}

export function fromDenseCoordinates(coords: DenselyPackedCoordinates): Coordinate[] {
    const length = getDenseLength(coords);
    const result: Coordinate[] = new Array(length);
    for (let n = 0; n < length; n++) {
        result[n] = [getDenseH(coords, n), getDenseI(coords, n)];
    }
    return result;
}
