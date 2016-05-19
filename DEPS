# This file is automatically processed to create .DEPS.git which is the file
# that gclient uses under git.
#
# See http://code.google.com/p/chromium/wiki/UsingGit
#
# To test manually, run:
#   python tools/deps2git/deps2git.py -o .DEPS.git -w <gclientdir>
# where <gcliendir> is the absolute path to the directory containing the
# .gclient file (the parent of 'motown').
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
  'buildtools_revision': '222bd42ce39d1bd8f08fe089b066f49c469e1cdf',
  'gurl_revision': '561639dfb664ee4c14371f569213b9d41f4fe110',
  'mojo_devtools_revision': '176889fd2e17f988727847a03b00c158af8a6c52',
  'mojo_sdk_revision': '3762c8c0b74b48b00786a42fca8f7db7f761935f',
}

# Only these hosts are allowed for dependencies in this DEPS file.
# If you need to add a new host, contact chrome infrastructure team.
allowed_hosts = [
  'chromium.googlesource.com',
]

deps = {
  'motown/base':
   Var('chromium_git') + '/external/github.com/domokit/base' + '@' +  Var('base_revision'),

  'motown/buildtools':
   Var('chromium_git') + '/chromium/buildtools.git' + '@' +  Var('buildtools_revision'),

  'motown/testing/gtest':
   Var('chromium_git') + '/external/googletest.git' + '@' + 'be1868139ffe0ccd0e8e3b37292b84c821d9c8ad', # from svn revision 704

  'motown/testing/gmock':
   Var('chromium_git') + '/external/googlemock.git' + '@' + '29763965ab52f24565299976b936d1265cb6a271', # from svn revision 501

  'motown/mojo/public':
   Var('chromium_git') + '/external/github.com/domokit/mojo_sdk.git' + '@' + Var('mojo_sdk_revision'),

  'motown/third_party/mojo_devtools':
  'https://github.com/domokit/devtools.git' + '@' + Var('mojo_devtools_revision'),

  'motown/third_party/icu':
   Var('chromium_git') + '/chromium/deps/icu.git' + '@' + '94e4b770ce2f6065d4261d29c32683a6099b9d93',

  'motown/third_party/yasm/source/patched-yasm':
   Var('chromium_git') + '/chromium/deps/yasm/patched-yasm.git' + '@' + '7da28c6c7c6a1387217352ce02b31754deb54d2a',

  'motown/third_party/ffmpeg':
     Var('chromium_git') + '/chromium/third_party/ffmpeg.git' + '@' + '6f7f37e8c16db3bad5624c7504e710c54bdb7bf5',

  'motown/url':
     Var('chromium_git') + '/external/github.com/domokit/gurl' + '@' +  Var('gurl_revision'),
}

deps_os = {
  'android': {
    'motown/third_party/jsr-305/src':
        Var('chromium_git') + '/external/jsr-305.git' + '@' + '642c508235471f7220af6d5df2d3210e3bfc0919',

    'motown/third_party/junit/src':
      Var('chromium_git') + '/external/junit.git' + '@' + '45a44647e7306262162e1346b750c3209019f2e1',

    'motown/third_party/mockito/src':
      Var('chromium_git') + '/external/mockito/mockito.git' + '@' + 'ed99a52e94a84bd7c467f2443b475a22fcc6ba8e',

    'motown/third_party/robolectric/lib':
      Var('chromium_git') + '/chromium/third_party/robolectric.git' + '@' + '6b63c99a8b6967acdb42cbed0adb067c80efc810',

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
        'motown/build/landmines.py',
    ],
  },
  {
    # Pull clang if needed or requested via GYP_DEFINES.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'motown/tools/clang/scripts/update.py', '--if-needed'],
  },
  {
    # This downloads android_tools according to tools/android/VERSION_*.
    'name': 'android_tools',
    'pattern': '.',
    'action': ['python', 'motown/tools/android/download_android_tools.py'],
  },
  {
    # This downloads SDK extras and puts them in the
    # third_party/android_tools/sdk/extras directory on the bots. Developers
    # need to manually install these packages and accept the ToS.
    'name': 'sdkextras',
    'pattern': '.',
    # When adding a new sdk extras package to download, add the package
    # directory and zip file to .gitignore in third_party/android_tools.
    'action': ['python', 'motown/build/download_sdk_extras.py'],
  },
  # Pull GN binaries. This needs to be before running GYP below.
  {
    'name': 'gn_linux64',
    'pattern': '.',
    'action': [ 'motown/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'motown/buildtools/linux64/gn.sha1',
    ],
  },
  {
    'name': 'gn_mac',
    'pattern': '.',
    'action': [ 'motown/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'motown/buildtools/mac/gn.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'action': [ 'motown/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'motown/buildtools/linux64/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'action': [ 'motown/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'motown/buildtools/mac/clang-format.sha1',
    ],
  },
  # Pull binutils for linux, enabled debug fission for faster linking /
  # debugging when used with clang on Ubuntu Precise.
  # https://code.google.com/p/chromium/issues/detail?id=352046
  {
    'name': 'binutils',
    'pattern': 'motown/third_party/binutils',
    'action': [
        'python',
        'motown/third_party/binutils/download.py',
    ],
  },
  # Pull the prebuilt network service binaries according to
  # mojo/public/tools/NETWORK_SERVICE_VERSION.
  {
    'name': 'download_network_service',
    'pattern': '',
    'action': [ 'python',
                'motown/mojo/public/tools/download_network_service.py',
                '--tools-directory', '../../../tools',
    ],
  },

  # Pull the mojom parser binaries using checked-in hashes.
  {
    'name': 'mojom_tool',
    'pattern': '',
    'action': [ 'motown/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'mojo/mojom_parser/linux64',
                '-s', 'motown/mojo/public/tools/bindings/mojom_tool/bin/linux64/mojom.sha1',
    ],
  },
  {
    'name': 'mojom_tool',
    'pattern': '',
    'action': [ 'motown/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'mojo/mojom_parser/mac64',
                '-s', 'motown/mojo/public/tools/bindings/mojom_tool/bin/mac64/mojom.sha1',
    ],
  },

  # Pull the mojom generator binaries using checked-in hashes.
  {
    'name': 'mojom_generators',
    'pattern': '',
    'action': [ 'motown/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'mojo/mojom_parser/linux64/generators',
                '-d', 'motown/mojo/public/tools/bindings/mojom_tool/bin/linux64/generators',
    ],
  },
  {
    'name': 'mojom_generators',
    'pattern': '',
    'action': [ 'motown/tools/download_from_google_storage.py',
                '--no_resume',
                '--quiet',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'mojo/mojom_parser/linux64/generators',
                '-d', 'motown/mojo/public/tools/bindings/mojom_tool/bin/linux64/generators',
    ],
  },
]
