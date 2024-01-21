# USAGE:
# Download Microsoft's ShaderConductor from here
# https://github.com/microsoft/ShaderConductor/releases/tag/v0.3.0
# Place the contents of the bin directory in the same directory as this python script
# Now you should be able to run this script, which should:
# 1. Open the shaders file for DX11 from the codebase
# 2. Parse each shader in it, and create a file for each, .ps or .vs
# 3. Run ShaderConductor on each shader file
# 4. Combine the output in a new shaders.h file for GL
# 5. Move the combined file back into the codebase

import re
import os

dx_shader_file = open("..\\helpers\\dx11\\shaders.h", 'r')
dx_shaders_cpp = dx_shader_file.read()
dx_shader_file.close()

op_path = "tmp\\"
if not os.path.exists(op_path):
	os.mkdir(op_path)

gl_shaders_file_name = "tmp\\shaders.h"
gl_shaders_file = open(gl_shaders_file_name, "w+") 
gl_shaders_file.write("#ifndef __WASTELADNS_SHADERS_GL_H__\n#define __WASTELADNS_SHADERS_GL_H__\n\n")

# grab every 'const char * = R("' variable with the shader code inside
shaders = re.findall(r"const\s+char\s*\*\s+([a-zA-Z0-9._]+)\s*=\s*R\"\(([^\"]*)\)\";", dx_shaders_cpp, re.DOTALL)
for shader in shaders: # [0] name [1] code

	gl_shaders_file.write("const char* " + shader[0] + " = R\"(\n")
	
	# write pixel or vertex shader to their own file for translation
	pixel_shader = True
	if re.search(r"[vV]ertex", shader[0]):
		pixel_shader = False
	shader_file_name = op_path + shader[0]+(".ps" if pixel_shader else ".vs")
	shader_file = open(shader_file_name, "w+")
	# ensure every matrix type specifies the column major order, so that ShaderConductor translates it correctly
	annotated_shader_code = re.sub(r"(\W)matrix(\s)", "\\1column_major matrix\\2", shader[1])
	annotated_shader_code = re.sub(r"(\W)float3x3(\s)", "\\1column_major float3x3\\2", annotated_shader_code)
	shader_file.write(annotated_shader_code)
	shader_file.close()

	if pixel_shader:
		# translate pixel shader
		gl_shader_file_name = op_path + shader[0] + "_ps.glsl"
		os.system("ShaderConductorCmd.exe -E \"PS\" -I \"" + shader_file_name + "\" -O \"" + gl_shader_file_name + "\" -S ps -T glsl -V \"330 core\"")
		gl_shader_file = open(gl_shader_file_name, 'r')
		gl_shader = gl_shader_file.read()
		# ensure inputs have the same name as vertex outputs
		gl_shader = gl_shader.replace('in_var_','varying_')
		gl_shader_file.close()
	else:
		# translate vertex shader
		gl_shader_file_name = op_path + shader[0] + "_vs.glsl"
		os.system("ShaderConductorCmd.exe -E \"VS\" -I \"" + shader_file_name + "\" -O \"" + gl_shader_file_name + "\" -S vs -T glsl -V \"330 core\"")
		gl_shader_file = open(gl_shader_file_name, 'r')
		gl_shader = gl_shader_file.read()
		# ensure outputs have the same name as pixel inputs
		gl_shader = gl_shader.replace('out_var_','varying_')
		gl_shader_file.close()
	
	gl_shaders_file.write(gl_shader)
	gl_shaders_file.write(")\";\n\n")

	
gl_shaders_file.write("\n\n#endif // __WASTELADNS_SHADERS_GL_H__\n")
gl_shaders_file.close()

os.replace(gl_shaders_file_name, "..\\helpers\\gl\\shaders.h")