# Contributing to SeaOS

Thank you for your interest in contributing to SeaOS! This document provides guidelines for contributing to the project.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/yourusername/seaos.git`
3. Create a feature branch: `git checkout -b feature-name`
4. Make your changes
5. Test your changes
6. Submit a pull request

## Development Environment

### Prerequisites
- GCC cross-compiler for x86_64
- NASM assembler
- GNU Make
- QEMU for testing
- Git for version control

### Setting Up
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install gcc-multilib nasm make qemu-system-x86

# Build the project
make pc

# Test in QEMU
make debug-pc
```

## Code Style

### C Code Style
- Use 4 spaces for indentation (no tabs)
- Maximum line length: 100 characters
- Function names: `snake_case`
- Variable names: `snake_case`
- Constants: `UPPER_CASE`
- Struct names: `snake_case_t`

### Example:
```c
#define MAX_BUFFER_SIZE 1024

typedef struct memory_region {
    uint64_t start_address;
    uint64_t size;
    uint32_t flags;
} memory_region_t;

static int allocate_memory_region(memory_region_t* region, size_t size) {
    if (!region || size == 0) {
        return -1;
    }
    
    // Implementation here
    return 0;
}
```

### Header Guards
Use `#ifndef` guards for all header files:
```c
#ifndef FILENAME_H
#define FILENAME_H

// Header content

#endif
```

## Platform-Specific Development

### Adding New Platform Support
1. Create platform directory: `kernel/platform/newplatform/`
2. Implement required functions:
   - `newplatform_platform_init()`
   - `newplatform_detect_hardware()`
   - Platform-specific subsystem initialization
3. Add platform detection in `platform_detect()`
4. Update `platform_get_capabilities()`
5. Add Makefile support

### Platform Directory Structure
```
kernel/platform/newplatform/
├── newplatform_init.c      # Main platform initialization
├── newplatform_power.c     # Power management
├── newplatform_display.c   # Display management
├── newplatform_input.c     # Input handling
└── newplatform_*.h         # Header files
```

## Driver Development

### Driver Interface
All drivers should implement a standard interface:
```c
// Driver initialization
int driver_init(void);

// Driver cleanup
void driver_cleanup(void);

// Driver-specific operations
int driver_operation(void* params);
```

### Driver Categories
- **GPU Drivers**: `kernel/drivers/gpu/`
- **Input Drivers**: `kernel/drivers/input/`
- **Network Drivers**: `kernel/drivers/net/`
- **Storage Drivers**: `kernel/drivers/storage/`
- **Audio Drivers**: `kernel/drivers/audio/`

## Testing

### Unit Testing
- Test individual functions and modules
- Use QEMU for kernel testing
- Test on all supported platforms

### Integration Testing
- Test platform-specific features
- Verify cross-platform compatibility
- Test hardware abstraction layer

### Testing Commands
```bash
# Test specific platform
make debug-pc
make debug-pe
make debug-tablet

# Run all tests
make test-all
```

## Documentation

### Code Documentation
- Document all public functions
- Use clear, descriptive comments
- Explain complex algorithms
- Document platform-specific behavior

### Example:
```c
/**
 * Allocates a contiguous block of physical memory
 * 
 * @param size Size in bytes to allocate
 * @param alignment Required alignment (must be power of 2)
 * @return Physical address of allocated memory, or 0 on failure
 * 
 * Note: This function may sleep and should not be called from
 * interrupt context.
 */
uint64_t allocate_physical_memory(size_t size, size_t alignment);
```

### README Updates
- Update README.md for new features
- Document new platform support
- Update capability matrix

## Commit Guidelines

### Commit Message Format
```
type(scope): brief description

Detailed description of changes (if needed)

- List specific changes
- Reference issues: Fixes #123
```

### Commit Types
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Test additions/changes
- `platform`: Platform-specific changes

### Examples
```
feat(tablet): add accelerometer sensor support

Add support for reading accelerometer data on tablet platform.
Includes calibration and orientation detection.

- Implement sensor initialization
- Add data reading functions
- Add orientation detection algorithm

Fixes #45
```

## Pull Request Process

1. **Create Feature Branch**: `git checkout -b feature-name`
2. **Make Changes**: Implement your feature/fix
3. **Test Changes**: Ensure all tests pass
4. **Update Documentation**: Update relevant documentation
5. **Commit Changes**: Follow commit guidelines
6. **Push Branch**: `git push origin feature-name`
7. **Create Pull Request**: Submit PR with description

### Pull Request Template
```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Platform support
- [ ] Documentation update

## Testing
- [ ] Tested on PC platform
- [ ] Tested on PE environment
- [ ] Tested on tablet platform
- [ ] Unit tests pass

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] No breaking changes
```

## Issue Reporting

### Bug Reports
Include:
- Platform affected
- Steps to reproduce
- Expected behavior
- Actual behavior
- System information
- Error messages/logs

### Feature Requests
Include:
- Platform relevance
- Use case description
- Proposed implementation
- Alternative solutions considered

## Code Review

### Review Criteria
- Code quality and style
- Platform compatibility
- Performance impact
- Security considerations
- Documentation completeness

### Review Process
1. Automated checks must pass
2. At least one maintainer review
3. Platform-specific review if applicable
4. Final approval and merge

## Release Process

### Version Numbering
- Major: Breaking changes
- Minor: New features
- Patch: Bug fixes

### Release Checklist
- [ ] All tests pass
- [ ] Documentation updated
- [ ] Platform compatibility verified
- [ ] Performance benchmarks
- [ ] Security review completed

## Community

### Communication
- GitHub Issues for bugs and features
- GitHub Discussions for questions
- Pull Requests for code contributions

### Code of Conduct
- Be respectful and inclusive
- Focus on constructive feedback
- Help newcomers learn
- Maintain professional communication

## Getting Help

### Resources
- README.md for basic information
- Code comments for implementation details
- GitHub Issues for specific problems
- GitHub Discussions for general questions

### Mentorship
New contributors are welcome! Maintainers are available to help with:
- Understanding the codebase
- Choosing appropriate first issues
- Code review and feedback
- Platform-specific development

Thank you for contributing to SeaOS!