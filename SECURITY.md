# BloodHorn Security Guide

This guide explains BloodHorn's security features and best practices for secure bootloader usage.

## Table of Contents

1. [Security Overview](#security-overview)
2. [Cryptographic Features](#cryptographic-features)
3. [Secure Boot Integration](#secure-boot-integration)
4. [TPM Support](#tpm-support)
5. [Kernel Verification](#kernel-verification)
6. [Configuration Security](#configuration-security)
7. [Network Security](#network-security)
8. [Best Practices](#best-practices)
9. [Troubleshooting](#troubleshooting)

## Security Overview

BloodHorn implements a comprehensive security model designed to protect the boot process from tampering and unauthorized modifications.

### Security Principles

- **Chain of Trust**: Each component verifies the next in the boot chain
- **Cryptographic Verification**: All critical components are cryptographically signed
- **Secure Storage**: Keys and certificates are stored securely
- **Audit Trail**: All security events are logged for analysis

### Threat Model

BloodHorn protects against:
- **Bootkit attacks**: Malicious code injected into boot process
- **Kernel tampering**: Unauthorized modifications to kernel images
- **Configuration attacks**: Malicious bootloader configurations
- **Network attacks**: Man-in-the-middle attacks during network boot
- **Physical attacks**: Direct hardware manipulation

## Cryptographic Features

### SHA256 Hashing

BloodHorn uses SHA256 for integrity verification of all critical components.

```c
// Example: Verify kernel integrity
uint8_t expected_hash[32] = {
    0x2c, 0x26, 0xb4, 0x6b, 0x68, 0xff, 0xc6, 0x8f,
    0xf9, 0x9b, 0x45, 0x3c, 0x23, 0x21, 0x51, 0x54,
    0xfd, 0x1b, 0xe8, 0xb7, 0x75, 0x81, 0x69, 0x2e,
    0xda, 0x91, 0x8d, 0x2f, 0x00, 0xca, 0x9d, 0x83
};

uint8_t actual_hash[32];
sha256_hash(kernel_data, kernel_size, actual_hash);

if (memcmp(expected_hash, actual_hash, 32) == 0) {
    // Kernel integrity verified
    boot_kernel(kernel_data);
} else {
    // Integrity check failed
    show_security_error("Kernel integrity verification failed");
}
```

### RSA Signature Verification

BloodHorn supports RSA-2048 and RSA-4096 signature verification for kernel and configuration files.

```bash
# Generate RSA key pair
openssl genrsa -out private.key 2048
openssl rsa -in private.key -pubout -out public.key

# Sign kernel
openssl dgst -sha256 -sign private.key -out kernel.sig kernel.bin

# Verify signature
openssl dgst -sha256 -verify public.key -signature kernel.sig kernel.bin
```

### Configuration

```ini
# /boot/bloodhorn-security.ini
[crypto]
hash_algorithm=sha256
signature_algorithm=rsa2048
verify_all_kernels=true
verify_config_files=true

[keys]
public_key_path=/boot/keys/public.key
allowed_key_hashes=sha256:abc123...def456
```

## Secure Boot Integration

### UEFI Secure Boot

BloodHorn integrates with UEFI Secure Boot to ensure only trusted code can execute.

```bash
# Check Secure Boot status
mokutil --sb-state

# Enroll BloodHorn certificate
sudo mokutil --import BloodHorn.der

# Verify BloodHorn is trusted
sudo efibootmgr -v | grep BloodHorn
```

### Certificate Management

```bash
# Generate BloodHorn certificate
openssl req -new -x509 -keyout BloodHorn.key -out BloodHorn.crt -days 365

# Convert to DER format for UEFI
openssl x509 -in BloodHorn.crt -outform DER -out BloodHorn.der

# Enroll in UEFI
sudo mokutil --import BloodHorn.der
```

### Configuration

```ini
# /boot/bloodhorn.ini
[secure_boot]
enabled=true
require_signed_kernels=true
require_signed_config=true
certificate_path=/boot/keys/BloodHorn.crt
```

## TPM Support

### Trusted Platform Module

BloodHorn supports TPM 2.0 for measured boot and secure key storage.

```c
// Example: Measure kernel before loading
uint8_t kernel_hash[32];
sha256_hash(kernel_data, kernel_size, kernel_hash);

// Extend PCR with kernel hash
tpm_extend_pcr(4, kernel_hash, 32);

// Store measurement in TPM
tpm_store_measurement("kernel", kernel_hash, 32);
```

### Measured Boot

```bash
# Check TPM measurements
sudo tpm2_pcrread sha256:0,1,2,3,4,5,6,7

# Verify measurements
sudo tpm2_policypcr -l sha256:4 -f pcr4.policy

# Quote TPM state
sudo tpm2_quote -c 0x81000000 -l sha256:4 -m quote.bin -s sig.bin
```

### Configuration

```ini
# /boot/bloodhorn.ini
[tpm]
enabled=true
measure_kernels=true
measure_config=true
pcr_bank=sha256
quote_on_boot=true
```

## Kernel Verification

### Automatic Verification

BloodHorn automatically verifies all kernel images before loading.

```bash
# Kernel verification process:
# 1. Load kernel from filesystem
# 2. Calculate SHA256 hash
# 3. Verify against stored hash
# 4. Verify RSA signature (if enabled)
# 5. Measure in TPM (if enabled)
# 6. Load and execute kernel
```

### Manual Verification

```bash
# Verify kernel manually
bloodhorn --verify-kernel /boot/vmlinuz-5.15.0-generic

# Check kernel signature
bloodhorn --verify-signature /boot/vmlinuz-5.15.0-generic /boot/kernel.sig

# List trusted kernels
bloodhorn --list-trusted-kernels
```

### Configuration

```ini
# /boot/bloodhorn.ini
[kernel_verification]
verify_all=true
require_signature=true
allowed_kernels=/boot/vmlinuz-5.15.0-generic,/boot/vmlinuz-5.15.0-generic.old
blocklist=/boot/vmlinuz-5.15.0-generic.broken
```

## Configuration Security

### Configuration Verification

BloodHorn verifies configuration files to prevent malicious modifications.

```bash
# Sign configuration file
openssl dgst -sha256 -sign private.key -out bloodhorn.ini.sig bloodhorn.ini

# Verify configuration
bloodhorn --verify-config /boot/bloodhorn.ini

# Check configuration integrity
bloodhorn --check-config-integrity
```

### Secure Configuration

```ini
# /boot/bloodhorn.ini
[security]
config_verification=true
config_signature_path=/boot/bloodhorn.ini.sig
allowed_config_paths=/boot/bloodhorn.ini,/boot/bloodhorn-local.ini
```

### Environment Variables

```bash
# Set secure environment
export BLOODHORN_SECURE_MODE=1
export BLOODHORN_VERIFY_ALL=1
export BLOODHORN_TPM_ENABLED=1

# Run BloodHorn with security
bloodhorn --secure-mode --verify-all --tpm-enabled
```

## Network Security

### PXE Security

BloodHorn implements security measures for network boot.

```bash
# Secure PXE configuration
[dhcp]
verify_server_certificate=true
require_server_authentication=true
allowed_servers=192.168.1.10,192.168.1.11

[tftp]
verify_file_integrity=true
require_file_signature=true
allowed_file_types=kernel,initrd,config
```

### HTTP Boot Security

```bash
# Secure HTTP boot
[http]
verify_ssl_certificate=true
require_https=true
allowed_servers=https://boot.example.com
verify_file_checksum=true
```

### Network Verification

```bash
# Verify network boot files
bloodhorn --verify-network-files

# Check server certificates
bloodhorn --verify-server-certificates

# Test network security
bloodhorn --test-network-security
```

## Best Practices

### Key Management

```bash
# Generate strong keys
openssl genrsa -out private.key 4096

# Store keys securely
sudo chmod 600 /boot/keys/private.key
sudo chown root:root /boot/keys/private.key

# Backup keys securely
sudo tar -czf keys-backup.tar.gz /boot/keys/
sudo gpg --encrypt keys-backup.tar.gz

# Rotate keys regularly
# Generate new keys every 6-12 months
```

### Certificate Management

```bash
# Create certificate hierarchy
# Root CA -> Intermediate CA -> BloodHorn Certificate

# Generate root CA
openssl genrsa -out root-ca.key 4096
openssl req -new -x509 -key root-ca.key -out root-ca.crt -days 3650

# Generate intermediate CA
openssl genrsa -out intermediate-ca.key 2048
openssl req -new -key intermediate-ca.key -out intermediate-ca.csr
openssl x509 -req -in intermediate-ca.csr -CA root-ca.crt -CAkey root-ca.key -out intermediate-ca.crt

# Generate BloodHorn certificate
openssl genrsa -out BloodHorn.key 2048
openssl req -new -key BloodHorn.key -out BloodHorn.csr
openssl x509 -req -in BloodHorn.csr -CA intermediate-ca.crt -CAkey intermediate-ca.key -out BloodHorn.crt
```

### Secure Deployment

```bash
# Secure deployment checklist:
# 1. Generate strong keys and certificates
# 2. Sign all kernels and configuration files
# 3. Enable Secure Boot
# 4. Configure TPM (if available)
# 5. Set up secure network boot (if needed)
# 6. Test all security features
# 7. Document security procedures
# 8. Train administrators
```

### Monitoring and Auditing

```bash
# Enable security logging
[logging]
security_events=true
log_level=debug
log_file=/var/log/bloodhorn-security.log

# Monitor security events
tail -f /var/log/bloodhorn-security.log

# Check security status
bloodhorn --security-status

# Generate security report
bloodhorn --security-report > security-report.txt
```

## Troubleshooting

### Common Security Issues

#### Secure Boot Errors

```bash
# Check Secure Boot status
mokutil --sb-state

# Re-enroll certificates
sudo mokutil --import BloodHorn.der

# Reset Secure Boot keys (if needed)
sudo mokutil --reset

# Check UEFI settings
sudo efibootmgr -v
```

#### TPM Issues

```bash
# Check TPM status
sudo tpm2_getcap properties-fixed

# Reset TPM (if needed)
sudo tpm2_clear

# Re-enroll TPM keys
sudo tpm2_createprimary -c primary.ctx
sudo tpm2_create -C primary.ctx -u key.pub -r key.priv
```

#### Signature Verification Failures

```bash
# Verify signature manually
openssl dgst -sha256 -verify public.key -signature kernel.sig kernel.bin

# Re-sign kernel
openssl dgst -sha256 -sign private.key -out kernel.sig kernel.bin

# Check key validity
openssl rsa -in private.key -check
```

### Recovery Procedures

#### Security Recovery

```bash
# Boot into recovery mode
# Select "Recovery Mode" from BloodHorn menu

# Verify system integrity
bloodhorn> verify-system-integrity

# Re-enroll security keys
bloodhorn> re-enroll-keys

# Reset security state
bloodhorn> reset-security-state
```

#### Emergency Access

```bash
# Emergency boot (bypass security)
# Press 'E' during boot to enter emergency mode

# Emergency configuration
[emergency]
bypass_security=true
allow_unsigned_kernels=true
emergency_timeout=30
```

### Security Testing

```bash
# Test security features
bloodhorn --test-security

# Penetration testing
# Test kernel tampering detection
# Test signature verification
# Test TPM measurements
# Test network security

# Security audit
bloodhorn --security-audit > audit-report.txt
```

This security guide covers the essential security features of BloodHorn. For advanced security configurations, refer to the source code and API documentation. 