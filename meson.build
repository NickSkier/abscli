project('abscli', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++20'])

abscli_sources = [
  'src/main.cpp',
  'src/db/statement.cpp',
  'src/db/db_manager.cpp',
  'src/network/http_client.cpp',
  'src/services/app_controller.cpp',
]

abscli_deps = [
  dependency('cpr'),
  dependency('sqlite3'),
  dependency('nlohmann_json'),
]

exe = executable('abscli', abscli_sources,
  dependencies: abscli_deps,
  install: true,
)

test('basic', exe)
