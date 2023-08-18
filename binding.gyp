{
  "targets": [
    {
      "includes": [ 
        "../native-huron/native_huron_files.gypi" 
      ],
      "target_name": "sdrjs",
      "sources": [ "src/sdrjs.cc", "src/device.cc", "<@(native_huron_files)" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "<!(node -e \"require('native-huron')\")"
      ],
      "conditions" : [
        [
          'OS!="win"', {
            "libraries" : [
              '-lrtlsdr',
            ],
          }
        ],
        [
          'OS=="win"', {
            "libraries" : [
              '<(module_root_dir)/gyp/lib/librtlsdr.dll.a'
            ]
          }
        ]
      ]
    }
  ]
}
