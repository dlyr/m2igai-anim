
#include <algorithm>
#include <chrono>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <glbinding/CallbackMask.h>
#include <glbinding/FunctionCall.h>
#include <glbinding/Version.h>
#include <glbinding/glbinding.h>

#include <glbinding/getProcAddress.h>
#include <glbinding/gl/gl.h>

#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/Meta.h>
#include <glbinding-aux/ValidVersions.h>
#include <glbinding-aux/debug.h>
#include <glbinding-aux/types_to_string.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <globjects/Program.h>
#include <globjects/Shader.h>
#include <globjects/Uniform.h>
#include <globjects/base/File.h>
#include <globjects/globjects.h>
#include <globjects/logging.h>

#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>

using namespace gl;
using namespace glm;
using namespace globjects;

class MiniMesh : public globjects::Instantiator<MiniMesh>
{
  public:
    struct Vertex {
        vec3 pos;
        vec3 normal;
    };
    using Face = std::array<gl::GLuint, 3>;

  public:
    MiniMesh( const std::vector<Vertex>& v, const std::vector<Face>& f );

    virtual ~MiniMesh() {};

    /** draws the mesh as single triangles
     */
    void draw();
    void draw( gl::GLenum mode );
    std::vector<Vertex>& vertices() { return m_vertices; }
    std::vector<Vertex>& restVertices() { return m_restVertices; }
    void updateVertices() {
        m_gpuVertices->setData( m_vertices, GL_STATIC_DRAW );
        auto vertexBinding = m_vao->binding( 0 );
        vertexBinding->setBuffer( m_gpuVertices.get(), 0, sizeof( Vertex ) );
    }

  protected:
    std::vector<Vertex> m_vertices;
    std::vector<Vertex> m_restVertices;
    std::vector<Face> m_indices;
    gl::GLsizei m_size;
    std::unique_ptr<globjects::VertexArray> m_vao;
    std::unique_ptr<globjects::Buffer> m_gpuVertices;
    std::unique_ptr<globjects::Buffer> m_gpuIndices;
};

MiniMesh::MiniMesh( const std::vector<Vertex>& v, const std::vector<Face>& f ) :
    m_vao( VertexArray::create() ),
    m_gpuVertices( Buffer::create() ),
    m_gpuIndices( Buffer::create() ) {
    m_indices.insert( m_indices.begin(), f.begin(), f.end() );
    m_restVertices.insert( m_vertices.begin(), v.begin(), v.end() );
    m_vertices.insert( m_vertices.begin(), v.begin(), v.end() );

    m_gpuIndices->setData( m_indices, GL_STATIC_DRAW );
    m_gpuVertices->setData( m_vertices, GL_STATIC_DRAW );

    m_size = static_cast<GLsizei>( m_indices.size() * 3 );

    m_vao->bindElementBuffer( m_gpuIndices.get() );

    {
        auto vertexBinding = m_vao->binding( 0 );
        vertexBinding->setAttribute( 0 );
        vertexBinding->setBuffer( m_gpuVertices.get(), 0, sizeof( Vertex ) );
        vertexBinding->setFormat( 3, GL_FLOAT, GL_FALSE );
        m_vao->enable( 0 );
    }
    {
        auto vertexBinding = m_vao->binding( 1 );
        vertexBinding->setAttribute( 1 );
        vertexBinding->setBuffer(
            m_gpuVertices.get(), offsetof( Vertex, normal ), sizeof( Vertex ) );
        vertexBinding->setFormat( 3, GL_FLOAT, GL_TRUE );
        m_vao->enable( 1 );
    }

    m_vao->unbind();
}

void MiniMesh::draw() {
    draw( GL_TRIANGLES );
}

void MiniMesh::draw( const GLenum mode ) {
    glEnable( GL_DEPTH_TEST );

    m_vao->bind();
    m_vao->drawElements( mode, m_size, GL_UNSIGNED_INT, nullptr );
    m_vao->unbind();
}

std::unique_ptr<MiniMesh> makeCylinder( vec3 base    = vec3( 0, 0, 0 ),
                                        vec3 axis    = vec3( 1, 0, 0 ),
                                        float radius = .5f,
                                        float length = 3.,
                                        int subdiv1  = 64,
                                        int subdiv2  = 512 ) {

    std::vector<MiniMesh::Vertex> vertices;
    std::vector<MiniMesh::Face> indices;
    vec3 x = vec3( 0, 1, 0 ); // orthogonal(axis);
    vec3 y = cross( axis, x );

    for ( int i = 0; i < subdiv2; i++ )
    {
        float offset  = float( i ) / float( subdiv2 - 1 );
        float offset2 = ( offset - 0.5 ) * length;
        for ( int j = 0; j < subdiv1; j++ )
        {
            float angle = 2. * glm::pi<float>() * float( j ) / float( subdiv1 );
            MiniMesh::Vertex nv;
            nv.pos = base + offset2 * axis + radius * cos( angle ) * x + radius * sin( angle ) * y;
            nv.normal = normalize( cos( angle ) * x + sin( angle ) * y );
            vertices.push_back( nv );
        }
    }

    for ( unsigned int i = 0; i < subdiv2 - 1; i++ )
    {
        for ( unsigned int j = 0; j < subdiv1; j++ )
        {
            MiniMesh::Face f1 {
                { i * subdiv1 + j, i * subdiv1 + ( j + 1 ) % subdiv1, i * subdiv1 + j + subdiv1 } };
            indices.push_back( f1 );
            MiniMesh::Face f2 { { i * subdiv1 + ( j + 1 ) % subdiv1,
                                  i * subdiv1 + j + subdiv1,
                                  i * subdiv1 + ( j + 1 ) % subdiv1 + subdiv1 } };
            indices.push_back( f2 );
        }
    }

    return MiniMesh::create( vertices, indices );
}

namespace {
std::unique_ptr<globjects::Program> g_program                             = nullptr;
std::unique_ptr<globjects::File> g_vertexShaderSource                     = nullptr;
std::unique_ptr<globjects::AbstractStringSource> g_vertexShaderTemplate   = nullptr;
std::unique_ptr<globjects::Shader> g_vertexShader                         = nullptr;
std::unique_ptr<globjects::File> g_fragmentShaderSource                   = nullptr;
std::unique_ptr<globjects::AbstractStringSource> g_fragmentShaderTemplate = nullptr;
std::unique_ptr<globjects::Shader> g_fragmentShader                       = nullptr;
std::unique_ptr<globjects::File> g_phongShaderSource                      = nullptr;
std::unique_ptr<globjects::AbstractStringSource> g_phongShaderTemplate    = nullptr;
std::unique_ptr<globjects::Shader> g_phongShader                          = nullptr;

std::unique_ptr<MiniMesh> g_mesh = nullptr;
glm::mat4 g_viewProjection;

const std::chrono::high_resolution_clock::time_point g_starttime =
    std::chrono::high_resolution_clock::now();

auto g_size = glm::ivec2 {};
} // namespace

void resize() {
    static const auto fovy   = glm::radians( 40.f );
    static const auto zNear  = 1.f;
    static const auto zFar   = 16.f;
    static const auto eye    = glm::vec3 { 0.f, 1.f, 8.f };
    static const auto center = glm::vec3 { 0.0, 0.0, 0.0 };
    static const auto up     = glm::vec3 { 0.0, 1.0, 0.0 };

    const auto aspect =
        static_cast<float>( g_size.x ) / glm::max( static_cast<float>( g_size.y ), 1.f );

    g_viewProjection =
        glm::perspective( fovy, aspect, zNear, zFar ) * glm::lookAt( eye, center, up );
}

void initialize() {
    const auto dataPath = std::string( "./Shaders/" );

    g_program = globjects::Program::create();

    g_vertexShaderSource = globjects::Shader::sourceFromFile( dataPath + "./basic.vert" );
    g_vertexShaderTemplate =
        globjects::Shader::applyGlobalReplacements( g_vertexShaderSource.get() );
    g_vertexShader = globjects::Shader::create( GL_VERTEX_SHADER, g_vertexShaderTemplate.get() );

    g_fragmentShaderSource = globjects::Shader::sourceFromFile( dataPath + "./basic.frag" );
    g_fragmentShaderTemplate =
        globjects::Shader::applyGlobalReplacements( g_fragmentShaderSource.get() );
    g_fragmentShader =
        globjects::Shader::create( GL_FRAGMENT_SHADER, g_fragmentShaderTemplate.get() );

    g_phongShaderSource   = globjects::Shader::sourceFromFile( dataPath + "./phong.frag" );
    g_phongShaderTemplate = globjects::Shader::applyGlobalReplacements( g_phongShaderSource.get() );
    g_phongShader = globjects::Shader::create( GL_FRAGMENT_SHADER, g_phongShaderTemplate.get() );

    g_program->attach( g_vertexShader.get(), g_fragmentShader.get(), g_phongShader.get() );

    g_mesh = makeCylinder();

    resize();
}

void deinitialize() {
    g_program.reset( nullptr );

    g_program.reset( nullptr );
    g_vertexShaderSource.reset( nullptr );
    g_vertexShaderTemplate.reset( nullptr );
    g_vertexShader.reset( nullptr );
    g_fragmentShaderSource.reset( nullptr );
    g_fragmentShaderTemplate.reset( nullptr );
    g_fragmentShader.reset( nullptr );
    g_phongShaderSource.reset( nullptr );
    g_phongShaderTemplate.reset( nullptr );
    g_phongShader.reset( nullptr );

    g_mesh.reset( nullptr );
}

void draw() {
    glClearColor( 0.01, 0.1, 0.1, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    const auto t_elapsed = std::chrono::high_resolution_clock::now() - g_starttime;
    const auto t         = static_cast<float>( t_elapsed.count() ) * 4e-10f;

    g_program->setUniform( "transform", g_viewProjection );

    const auto level = static_cast<int>( ( sin( t ) * 0.5f + 0.5f ) * 16 ) + 1;
    g_program->setUniform( "level", level );
    g_program->use();

    glViewport( 0, 0, g_size.x, g_size.y );

    g_mesh->draw( GL_TRIANGLES );

    g_program->release();
}

void error( int errnum, const char* errmsg ) {
    globjects::critical() << errnum << ": " << errmsg << std::endl;
}

void framebuffer_size_callback( GLFWwindow* /*window*/, int width, int height ) {
    g_size = glm::ivec2 { width, height };
    resize();
}

void key_callback( GLFWwindow* window, int key, int /*scancode*/, int action, int /*modes*/ ) {
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE )
        glfwSetWindowShouldClose( window, true );

    if ( key == GLFW_KEY_F5 && action == GLFW_RELEASE )
    {
        g_vertexShaderSource->reload();
        g_fragmentShaderSource->reload();
        g_phongShaderSource->reload();
    }
}

int main( int /*argc*/, char* /*argv*/[] ) {
    // Initialize GLFW
    if ( !glfwInit() ) return 1;

    glfwSetErrorCallback( error );
    glfwDefaultWindowHints();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, true );

    // Create a context and, if valid, make it current
    GLFWwindow* window = glfwCreateWindow( 640, 480, "globjects Tessellation", NULL, NULL );
    if ( window == nullptr )
    {
        globjects::critical() << "Context creation failed. Terminate execution.";

        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback( window, key_callback );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

    glfwMakeContextCurrent( window );

    // Initialize globjects (internally initializes glbinding, and registers the current context)
    globjects::init( []( const char* name ) { return glfwGetProcAddress( name ); } );

    std::cout << std::endl
              << "OpenGL Version:  " << glbinding::aux::ContextInfo::version() << std::endl
              << "OpenGL Vendor:   " << glbinding::aux::ContextInfo::vendor() << std::endl
              << "OpenGL Renderer: " << glbinding::aux::ContextInfo::renderer() << std::endl;

    globjects::info() << "Press F5 to reload shaders." << std::endl << std::endl;

    glfwGetFramebufferSize( window, &g_size[0], &g_size[1] );
    initialize();

    // Main loop
    while ( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();
        draw();
        glfwSwapBuffers( window );
    }
    deinitialize();

    // Properly shutdown GLFW
    glfwTerminate();

    return 0;
}
