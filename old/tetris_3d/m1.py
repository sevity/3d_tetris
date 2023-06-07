def write_outline_I_block(filename):
    with open(filename, 'w') as file:
        # Define vertices
        for i in range(2):  # For each x coordinate (0 and 1)
            for j in range(2):  # For each y coordinate (0 and 1)
                for l in range(5):  # For each z coordinate (0, 1, 2, 3, and 4)
                    file.write(f"v {i} {j} {l}\n")

        # Define lines
        # 4 lines on the top face
        for i in range(4):  # For each pair of adjacent vertices
            file.write(f"l {1 + i} {1 + ((i + 1) % 4)}\n")
        # 4 lines on the bottom face
        for i in range(4):  # For each pair of adjacent vertices
            file.write(f"l {17 + i} {17 + ((i + 1) % 4)}\n")
        # 4 lines on the side faces
        for i in range(4):
            file.write(f"l {1 + 4*i} {5 + 4*i}\n")
            file.write(f"l {2 + 4*i} {6 + 4*i}\n")
            file.write(f"l {3 + 4*i} {7 + 4*i}\n")
            file.write(f"l {4 + 4*i} {8 + 4*i}\n")

write_outline_I_block("I_block.obj")

