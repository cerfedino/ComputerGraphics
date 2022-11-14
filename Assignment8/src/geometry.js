//------------ Assignment ----------------
// Add vertices for the cube as well as colors for corresponding vertices.
// Remember that:
// (1) The vertices for each triangle has to be provided in a counter-clockwise order to make the face culling work properly. If the order of vertices won't be correct the triangles won't be visible.
// (2) The current coordinate system has: x-axis pointing right, y-axis pointing up, and z-axis pointing towards the screen
// (3) Everything which is within [-1..1]x[-1..1]x[-1..1] volume will be projected onto the screen along z-axis
//-----------------------------------------


function hrect(x, y, z, deltaX = 0, deltaY = 0) {
    const p1 = [x + deltaX / 2, y + deltaY / 2, z - deltaZ / 2]
    const p2 = [x - deltaX / 2, y + deltaY / 2, z + deltaZ / 2,]
    const p3 = [x - deltaX / 2, y - deltaY / 2, z + deltaZ / 2,]
    const p4 = [x + deltaX / 2, y - deltaY / 2, z - deltaZ / 2]
    return [
        ...p1, ...p2, ...p3,
        ...p1, ...p3, ...p4
    ]
}

// var rotationMatrix = mat4.create();
// mat4.fromRotation(rotationMatrix, -(0.5) / 100 * Math.PI, vec3.fromValues(-0.2, 1, 0));

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

function reverseMesh(vertices) {
    let result = []
    for(let i=0; i < vertices.length; i +=3) {
        result.push(vertices.slice(i, i+3))
    }
    return result.reverse().flat()
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

const RED = [1, 0, 0]
const GREEN = [0, 1, 0]
const BLUE = [0, 0, 1]
const YELLOW = [1, 1, 0];
const MAGENTA = [1, 0, 1];
const CYAN = [0, 1, 1];