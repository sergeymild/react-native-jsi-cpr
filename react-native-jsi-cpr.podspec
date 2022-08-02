require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

Pod::Spec.new do |s|
  s.name         = "react-native-jsi-cpr"
  s.version      = package["version"]
  s.summary      = package["description"]
  s.homepage     = package["homepage"]
  s.license      = package["license"]
  s.authors      = package["author"]

  s.platforms    = { :ios => "11.0" }
  s.source       = { :git => "https://github.com/sergeymild/react-native-jsi-cpr.git", :tag => "#{s.version}" }

  s.source_files = "ios/**/*.{h,m,mm}", "cpp/**/*.{h,cpp}"

  s.subspec 'cpr' do |ali|
     ali.source_files = 'cpp/include/**/*.{h,cpp}'
     ali.header_mappings_dir = 'cpp/include'
  end

  s.dependency "React-Core"
  s.dependency "React-callinvoker"
  s.dependency 'ReactCommon/turbomodule/core'
  s.dependency 'curl'
end
