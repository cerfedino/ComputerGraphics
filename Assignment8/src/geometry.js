/*
    All functions to build shapes
 */
const RED = [1, 0, 0]
const GREEN = [0, 1, 0]
const BLUE = [0, 0, 1]
const YELLOW = [1, 1, 0];
const MAGENTA = [1, 0, 1];
const CYAN = [0, 1, 1];


function triangle(x, y, z, size) {
    return {
        vertices: [
            0.0, size/2, 0.0,
            -size/2, -size/2, 0.0,
            size/2, -size/2, 0.0,
        ],
        colors: [RED, GREEN, BLUE].flat()
    }
}


function cube(x, y, z, size) {
    return {
        vertices: [
            // FRONT
            ...squareAlongXY(0,0,-size/2,size),
            // BACK
            ...reverseMesh(squareAlongXY(0,0,size/2,size)),

            // LEFT
            ... squareAlongYZ(-size/2,0,0,size),
            // RIGHT
            ...reverseMesh(squareAlongYZ(size/2,0,0,size)),

            // TOP
            ...squareAlongXZ(0,size/2,0,size),
            // BOTTOM
            ...reverseMesh(squareAlongXZ(0,-size/2,0,size))
        ],

        // One color per face
        colors: [RED, GREEN, BLUE, YELLOW, MAGENTA, CYAN].reduce((acc, c) => acc.concat(c, c, c, c, c, c), []),
    }
}


// Square with the face pointing towards the camera
function squareAlongXY(x, y, z, size) {
    const p1 = [x + size / 2, y + size / 2, z]
    const p2 = [x - size / 2, y + size / 2, z]
    const p3 = [x - size / 2, y - size / 2, z]
    const p4 = [x + size / 2, y - size / 2, z]
    return [
        ...p1, ...p2, ...p3,
        ...p1, ...p3, ...p4
    ]
}
// Square with the face pointing upwards
function squareAlongYZ(x, y, z, size) {
    const p1 = [x, y + size / 2, z - size / 2]
    const p2 = [x, y + size / 2, z + size / 2,]
    const p3 = [x, y - size / 2, z + size / 2,]
    const p4 = [x, y - size / 2, z - size / 2]
    return [
        ...p1, ...p2, ...p3,
        ...p1, ...p3, ...p4
    ]
}
// Square with face pointing left
function squareAlongXZ(x, y, z, size) {
    const p1 = [x + size / 2, y, z + size / 2]
    const p2 = [x - size / 2, y, z + size / 2]
    const p3 = [x - size / 2, y, z - size / 2]
    const p4 = [x + size / 2, y, z - size / 2]
    return [
        ...p1, ...p2, ...p3,
        ...p1, ...p3, ...p4
    ]
}
// Reverses a mesh composed by triangles.
function reverseMesh(vertices) {
    let result = []
    for(let i=0; i < vertices.length; i +=3) {
        result.push(vertices.slice(i, i+3))
    }
    return result.reverse().flat()
}