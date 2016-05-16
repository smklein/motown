# This file is automatically processed to create .DEPS.git which is the file
# that gclient uses under git.
#
# See http://code.google.com/p/chromium/wiki/UsingGit
#
# To test manually, run:
#   python tools/deps2git/deps2git.py -o .DEPS.git -w <gclientdir>
# where <gcliendir> is the absolute path to the directory containing the
# .gclient file (the parent of 'src').
#
# Then commit .DEPS.git locally (gclient doesn't like dirty trees) and run
#   gclient sync
# Verify the thing happened you wanted. Then revert your .DEPS.git change
# DO NOT CHECK IN CHANGES TO .DEPS.git upstream. It will be automatically
# updated by a bot when you modify this one.
#
# When adding a new dependency, please update the top-level .gitignore file
# to list the dependency's destination directory.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'base_revision': 'f613c71b2ed7fe4b4eff33fb7fd3b53e640b4359',
  'angle_revision': '46ccef1992a8ede16a596c3dd73cff13c047267d',
  'buildtools_revision': '222bd42ce39d1bd8f08fe089b066f49c469e1cdf',
  'dart_revision': 'e4db01fdd43f019988a901eb51c72790652760a4',
  'dart_root_certificates_revision': 'aed07942ce98507d2be28cbd29e879525410c7fc',
  'dart_observatory_packages_revision': 'cf90eb9077177d3d6b3fd5e8289477c2385c026a',
  'boringssl_revision': '642f1498d056dbba3e50ed5a232ab2f482626dec',
  'lss_revision': '6f97298fe3794e92c8c896a6bc06e0b36e4c3de3',
  'tonic_revision': 'e41f72c684e0da61304ebd6f3147d0fe23ffbb56',
  'gurl_revision': '561639dfb664ee4c14371f569213b9d41f4fe110',
}

# Only these hosts are allowed for dependencies in this DEPS file.
# If you need to add a new host, contact chrome infrastructure team.
allowed_hosts = [
  'boringssl.googlesource.com',
  'chromium.googlesource.com',
]

deps = {
  'src/base':
   Var('chromium_git') + '/external/github.com/domokit/base' + '@' +  Var('base_revision'),

  'src/buildtools':
   Var('chromium_git') + '/chromium/buildtools.git' + '@' +  Var('buildtools_revision'),

  'src/testing/gtest':
   Var('chromium_git') + '/external/googletest.git' + '@' + 'be1868139ffe0ccd0e8e3b37292b84c821d9c8ad', # from svn revision 704

  'src/testing/gmock':
   Var('chromium_git') + '/external/googlemock.git' + '@' + '29763965ab52f24565299976b936d1265cb6a271', # from svn revision 501

  'src/third_party/angle':
   Var('chromium_git') + '/angle/angle.git' + '@' +  Var('angle_revision'),

  'src/third_party/icu':
   Var('chromium_git') + '/chromium/deps/icu.git' + '@' + '94e4b770ce2f6065d4261d29c32683a6099b9d93',

  'src/tools/grit':
    Var('chromium_git') + '/external/grit-i18n.git' + '@' + 'c1b1591a05209c1ad467e845ba8543c22f9072af', # from svn revision 189

  'src/dart':
    Var('chromium_git') + '/external/github.com/dart-lang/sdk.git' + '@' + Var('dart_revision'),

  'src/tonic':
    Var('chromium_git') + '/external/github.com/domokit/tonic' + '@' + Var('tonic_revision'),

  'src/dart/third_party/observatory_pub_packages':
    Var('chromium_git') +
    '/external/github.com/dart-lang/observatory_pub_packages' + '@' +
    Var('dart_observatory_packages_revision'),

  'src/dart/third_party/root_certificates':
    Var('chromium_git') +
    '/external/github.com/dart-lang/root_certificates' + '@' +
    Var('dart_root_certificates_revision'),

  'src/third_party/glm':
    Var('chromium_git') + '/external/github.com/g-truc/glm' + '@' + '93d09e0e93ca6fe3d9dc6398489a54a3f9cf76db',

  'src/third_party/yasm/source/patched-yasm':
   Var('chromium_git') + '/chromium/deps/yasm/patched-yasm.git' + '@' + '7da28c6c7c6a1387217352ce02b31754deb54d2a',

  'src/third_party/smhasher/src':
    Var('chromium_git') + '/external/smhasher.git' + '@' + 'e87738e57558e0ec472b2fc3a643b838e5b6e88f',

  'src/third_party/pywebsocket/src':
    Var('chromium_git') + '/external/pywebsocket/src.git' + '@' + 'cb349e87ddb30ff8d1fa1a89be39cec901f4a29c',

  'src/third_party/mesa/src':
   Var('chromium_git') + '/chromium/deps/mesa.git' + '@' + '071d25db04c23821a12a8b260ab9d96a097402f0',

  'src/third_party/boringssl/src':
   'https://boringssl.googlesource.com/boringssl.git' + '@' +  Var('boringssl_revision'),

  'src/third_party/requests/src':
    Var('chromium_git') + '/external/github.com/kennethreitz/requests.git' + '@' + 'f172b30356d821d180fa4ecfa3e71c7274a32de4',

  'src/third_party/pyelftools':
    Var('chromium_git') + '/chromiumos/third_party/pyelftools.git' + '@' + '19b3e610c86fcadb837d252c794cb5e8008826ae',

  'src/third_party/breakpad/src':
    Var('chromium_git') + '/external/google-breakpad/src.git' + '@' + '242fb9a38db6ba534b1f7daa341dd4d79171658b', # from svn revision 1471

  'src/third_party/lss':
    Var('chromium_git') + '/external/linux-syscall-support/lss.git' + '@' + Var('lss_revision'),

  'src/third_party/leveldatabase/src':
    Var('chromium_git') + '/external/leveldb.git' + '@' + '40c17c0b84ac0b791fb434096fd5c05f3819ad55',

  'src/third_party/snappy/src':
    Var('chromium_git') + '/external/snappy.git' + '@' + '762bb32f0c9d2f31ba4958c7c0933d22e80c20bf',

  'src/third_party/ffmpeg':
     Var('chromium_git') + '/chromium/third_party/ffmpeg.git' + '@' + '6f7f37e8c16db3bad5624c7504e710c54bdb7bf5',

  'src/third_party/libcxx/libcxx':
     Var('chromium_git') + '/chromium/llvm-project/libcxx.git' + '@' + '2dcc9a932e33ac4228eedcb6e026ac480daa8e45',

  'src/third_party/libcxx/libcxxabi':
     Var('chromium_git') + '/chromium/llvm-project/libcxxabi.git' + '@' + '3a1fd0deeabcefb42463eb0ac9a570140679e605',

  'src/url':
     Var('chromium_git') + '/external/github.com/domokit/gurl' + '@' +  Var('gurl_revision'),
}

deps_os = {
  'android': {
    'src/third_party/colorama/src':
     Var('chromium_git') + '/external/colorama.git' + '@' + '799604a1041e9b3bc5d2789ecbd7e8db2e18e6b8',

    'src/third_party/jsr-305/src':
        Var('chromium_git') + '/external/jsr-305.git' + '@' + '642c508235471f7220af6d5df2d3210e3bfc0919',

    'src/third_party/junit/src':
      Var('chromium_git') + '/external/junit.git' + '@' + '45a44647e7306262162e1346b750c3209019f2e1',

    'src/third_party/mockito/src':
      Var('chromium_git') + '/external/mockito/mockito.git' + '@' + 'ed99a52e94a84bd7c467f2443b475a22fcc6ba8e',

    'src/third_party/robolectric/lib':
      Var('chromium_git') + '/chromium/third_party/robolectric.git' + '@' + '6b63c99a8b6967acdb42cbed0adb067c80efc810',

    'src/third_party/appurify-python/src':
     Var('chromium_git') + '/external/github.com/appurify/appurify-python.git' + '@' + 'ee7abd5c5ae3106f72b2a0b9d2cb55094688e867',

    'src/third_party/freetype-android/src':
       Var('chromium_git') + '/chromium/src/third_party/freetype.git' + '@' + 'd1028db70bea988d1022e4d463de66581c696160',

    'src/third_party/requests/src':
      Var('chromium_git') + '/external/github.com/kennethreitz/requests.git' + '@' + 'f172b30356d821d180fa4ecfa3e71c7274a32de4',

    'src/third_party/pyelftools':
     Var('chromium_git') + '/chromiumos/third_party/pyelftools.git' + '@' + '19b3e610c86fcadb837d252c794cb5e8008826ae',

  },
}


hooks = [
  {
    # This clobbers when necessary (based on get_landmines.py). It must be the
    # first hook so that other things that get/generate into the output
    # directory will not subsequently be clobbered.
    'name': 'landmines',
    'pattern': '.',
    'action': [
        'python',
        'src/build/landmines.py',
    ],
  },
  {
    # Pull clang if needed or requested via GYP_DEFINES.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'src/tools/clang/scripts/update.py', '--if-needed'],
  },
  {
    # Pull dart sdk if needed
    'name': 'dart',
    'pattern': '.',
    'action': ['python', 'src/tools/dart/update.py'],
  },
  {
    # This downloads android_tools according to tools/android/VERSION_*.
    'name': 'android_tools',
    'pattern': '.',
    'action': ['python', 'src/tools/android/download_android_tools.py'],
  },
  {
    # This downloads SDK extras and puts them in the
    # third_party/android_tools/sdk/extras directory on the bots. Developers
    # need to manually install these packages and accept the ToS.
    'name': 'sdkextras',
    'pattern': '.',
    # When adding a new sdk extras package to download, add the package
    # directory and zip file to .gitignore in third_party/android_tools.
    'action': ['python', 'src/build/download_sdk_extras.py'],
  },
  # Pull GN binaries. This needs to be before running GYP below.
  {
    'name': 'gn_linux64',
    'pattern': '.',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/linux64/gn.sha1',
    ],
  },
  {
    'name': 'gn_mac',
    'pattern': '.',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/mac/gn.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/linux64/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/mac/clang-format.sha1',
    ],
  },
  # Pull binutils for linux, enabled debug fission for faster linking /
  # debugging when used with clang on Ubuntu Precise.
  # https://code.google.com/p/chromium/issues/detail?id=352046
  {
    'name': 'binutils',
    'pattern': 'src/third_party/binutils',
    'action': [
        'python',
        'src/third_party/binutils/download.py',
    ],
  },
  # Pull the prebuilt network service binaries according to
  # mojo/public/tools/NETWORK_SERVICE_VERSION.
  {
    'name': 'download_network_service',
    'pattern': '',
    'action': [ 'python',
                'src/mojo/public/tools/download_network_service.py',
                '--tools-directory', '../../../tools',
    ],
  },
  {
    # Ensure that we don't accidentally reference any .pyc files whose
    # corresponding .py files have already been deleted.
    'name': 'remove_stale_pyc_files',
    'pattern': 'src/tools/.*\\.py',
    'action': [
        'python',
        'src/tools/remove_stale_pyc_files.py',
        'src/tools',
    ],
  },
  {
    # This downloads linux and android Go binaries according to
    # tools/go/VERSION.
    'name': 'gotools',
    'pattern': '.',
    'action': [
        'python', 'src/tools/go/download.py',
    ],
  },
  # Pull dump_syms resources using checked-in hashes.
  {
    'name': 'dump_syms_linux64',
    'pattern': '.',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'mojo',
                '-s', 'src/mojo/tools/linux64/dump_syms.sha1',
    ],
  },
  # Pull symupload resources using checked-in hashes.
  {
    'name': 'symupload_linux64',
    'pattern': '.',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'mojo',
                '-s', 'src/mojo/tools/linux64/symupload.sha1',
	],
  },
  # Pull prediction resources using checked-in hashes.
  {
    'name': 'prediction_resources',
    'pattern': '',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--no_auth',
                '--bucket', 'mojo/prediction',
                '-d', 'src/services/prediction/res',
    ],
  },
  # Pull the mojom parser binaries using checked-in hashes.
  {
    'name': 'mojom_tool',
    'pattern': '',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'mojo/mojom_parser/linux64',
                '-s', 'src/mojo/public/tools/bindings/mojom_tool/bin/linux64/mojom.sha1',
    ],
  },
  {
    'name': 'mojom_tool',
    'pattern': '',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'mojo/mojom_parser/mac64',
                '-s', 'src/mojo/public/tools/bindings/mojom_tool/bin/mac64/mojom.sha1',
    ],
  },

  # Pull the mojom generator binaries using checked-in hashes.
  {
    'name': 'mojom_generators',
    'pattern': '',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'mojo/mojom_parser/linux64/generators',
                '-d', 'src/mojo/public/tools/bindings/mojom_tool/bin/linux64/generators',
    ],
  },
  {
    'name': 'mojom_generators',
    'pattern': '',
    'action': [ 'src/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'mojo/mojom_parser/linux64/generators',
                '-d', 'src/mojo/public/tools/bindings/mojom_tool/bin/linux64/generators',
    ],
  },
]
