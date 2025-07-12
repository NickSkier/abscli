project('abscli', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++17'])

abscli_sources = [
  'src/abscli.cpp',
  'src/network/http_client.cpp',
]

abscli_deps = [
  dependency('cpr'),
  dependency('nlohmann_json'),
]

exe = executable('abscli', abscli_sources,
  dependencies: abscli_deps,
  install: true,
)

test('basic', exe)
