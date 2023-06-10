
def write_cube(file, x, y, z):
    file.write(f"v {x} {y} {z}\n")
    file.write(f"v {x+1} {y} {z}\n")
    file.write(f"v {x+1} {y} {z+1}\n")
    file.write(f"v {x} {y} {z+1}\n")
    file.write(f"v {x} {y+1} {z}\n")
    file.write(f"v {x+1} {y+1} {z}\n")
    file.write(f"v {x+1} {y+1} {z+1}\n")
    file.write(f"v {x} {y+1} {z+1}\n")


def generate_custom_block_obj(filename, block_shape):
    with open(filename, 'w') as file:
        base_idx = 0

        for z in range(3):
            for y in range(3):
                for x in range(3):
                    if block_shape[z][y][x] == 1:
                        write_cube(file, x, y, z)
                        # bottom face
                        file.write(f"f {base_idx+1} {base_idx+2} {base_idx+3} {base_idx+4}\n")
                        # top face
                        file.write(f"f {base_idx+5} {base_idx+6} {base_idx+7} {base_idx+8}\n")
                        # front face
                        file.write(f"f {base_idx+1} {base_idx+2} {base_idx+6} {base_idx+5}\n")
                        # back face
                        file.write(f"f {base_idx+3} {base_idx+4} {base_idx+8} {base_idx+7}\n")
                        # left face
                        file.write(f"f {base_idx+1} {base_idx+4} {base_idx+8} {base_idx+5}\n")
                        # right face
                        file.write(f"f {base_idx+2} {base_idx+3} {base_idx+7} {base_idx+6}\n")
                        base_idx += 8

block_shape = [
    [[1,1,0],[0,0,0],[0,0,0]],
    [[0,1,0],[0,1,0],[0,0,0]],
    [[0,0,0],[0,0,0],[0,0,0]]
]
generate_custom_block_obj("d_block.obj", block_shape)


block_shape = [
    [[1,1,0],[0,0,0],[0,0,0]],
    [[0,1,0],[0,0,0],[0,0,0]],
    [[0,0,0],[0,0,0],[0,0,0]]
]
generate_custom_block_obj("n_block.obj", block_shape)

block_shape = [
    [[1,1,0],[0,0,0],[0,0,0]],
    [[0,1,0],[0,0,0],[0,0,0]],
    [[0,1,0],[0,0,0],[0,0,0]]
]
generate_custom_block_obj("l_block.obj", block_shape)


block_shape = [
    [[1,1,0],[0,0,0],[0,0,0]],
    [[1,1,0],[0,0,0],[0,0,0]],
    [[0,0,0],[0,0,0],[0,0,0]]
]
generate_custom_block_obj("o_block.obj", block_shape)

block_shape = [
    [[0,0,0],[0,0,0],[1,0,0]],
    [[0,0,0],[0,0,0],[1,1,0]],
    [[0,0,0],[0,0,0],[0,1,0]]
]
generate_custom_block_obj("s_block.obj", block_shape)

block_shape = [
    [[0,0,0],[0,0,0],[0,0,0]],
    [[0,0,0],[0,0,0],[0,0,0]],
    [[0,0,0],[0,1,0],[1,1,1]]
]
generate_custom_block_obj("t_block.obj", block_shape)

