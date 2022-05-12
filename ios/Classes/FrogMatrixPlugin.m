#import "FrogMatrixPlugin.h"
#if __has_include(<frog_matrix_plugin/frog_matrix_plugin-Swift.h>)
#import <frog_matrix_plugin/frog_matrix_plugin-Swift.h>
#else
// Support project import fallback if the generated compatibility header
// is not copied when this plugin is created as a library.
// https://forums.swift.org/t/swift-static-libraries-dont-copy-generated-objective-c-header/19816
#import "frog_matrix_plugin-Swift.h"
#endif

@implementation FrogMatrixPlugin
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  [SwiftFrogMatrixPlugin registerWithRegistrar:registrar];
}
@end
