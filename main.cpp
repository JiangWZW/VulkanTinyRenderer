#include "pch.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "TinyVulkanRenderer.h"
#include "SwapChainConfigurator.h"

// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
typedef OpenMesh::TriMesh_ArrayKernelT<>  MyMesh;

#include <OpenMesh/Core/IO/MeshIO.hh>
MyMesh mesh;
// ----------------------------------------------------------------------------


GLFWwindow* window;
TinyVulkanRenderer m_renderer;


int main() {
	// --------------------------------------------------------------
	// Load mesh data
	// --------------------------------------------------------------
	if (!OpenMesh::IO::read_mesh(mesh, "Assets/bunny.obj"))
	{
		std::cerr << "read error\n";
		exit(1);
	}
	for (MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it)
	{
		
	}
	// do something with your mesh ...
	if (!OpenMesh::IO::write_mesh(mesh, "Assets/rosa02.obj"))
	{
		std::cerr << "write error\n";
		exit(1);
	}
	mesh.points();

	
	

	
	// --------------------------------------------------------------
	// Render loop
	// --------------------------------------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(
		800, 600, 
		"Vulkan Window", 
		nullptr, 
		nullptr
	);
	m_renderer.SetWindow(window);
	
	m_renderer.InitVkObjects();
	
	
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		m_renderer.Draw();
	}
	
	
	m_renderer.DestroyVkObjects();
	
	glfwDestroyWindow(window);
	glfwTerminate();
	
}