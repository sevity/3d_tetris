def generate_obj_file(filename):
    with open(filename, 'w') as file:
        # Vertices and faces for the bottom grid
        for i in range(4):
            for j in range(4):
                file.write(f"v {i} {j} 10\n")  # Swap y and z
        for i in range(3):
            for j in range(3):
                idx = i * 4 + j + 1
                file.write(f"f {idx} {idx+1} {idx+5} {idx+4}\n")

        # Vertices and faces for the walls
        base_idx = 16
        for k in range(4):  # For each wall
            for i in range(4):  # For each row
                for j in range(11):  # For each column
                    if k == 0:
                        file.write(f"v 0 {i} {j}\n")  # Swap y and z
                    elif k == 1:
                        file.write(f"v {i} 3 {j}\n")  # Swap y and z
                    elif k == 2:
                        file.write(f"v 3 {i} {j}\n")  # Swap y and z
                    elif k == 3:
                        file.write(f"v {i} 0 {j}\n")  # Swap y and z
            for i in range(3):  # For each row
                for j in range(10):  # For each column
                    idx = base_idx + i * 11 + j + 1
                    file.write(f"f {idx} {idx+1} {idx+12} {idx+11}\n")
            base_idx += 44

generate_obj_file("stage.obj")

