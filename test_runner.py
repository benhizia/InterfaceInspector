#!/usr/bin/env python3
"""
Test runner for header_inspector.py
Processes all test header files and generates reports
"""

import os
import sys
import subprocess
import json
import time
from pathlib import Path

def run_header_inspector(header_path):
    """Run header inspector on a single file and return success/failure info"""
    try:
        print(f"  Processing {header_path.name}...")
        
        # Run the header inspector using the run.sh script
        result = subprocess.run([
            "./run.sh", str(header_path)
        ], capture_output=True, text=True, timeout=30)
        
        if result.returncode == 0:
            # Check if output files were created in MetaDataGen
            output_dir = Path("MetaDataGen")
            base_path = header_path.with_suffix("")
            json_file = output_dir / base_path.with_suffix(".meta.json").name
            hpp_file = output_dir / base_path.with_suffix(".meta.hpp").name
            
            json_exists = json_file.exists()
            hpp_exists = hpp_file.exists()
            
            # Try to load and validate JSON if it exists
            json_valid = False
            json_size = 0
            if json_exists:
                try:
                    with open(json_file, 'r') as f:
                        data = json.load(f)
                        json_valid = isinstance(data, dict)
                        json_size = len(json.dumps(data))
                except Exception as e:
                    print(f"    ⚠️  JSON validation failed: {e}")
            
            return {
                "success": True,
                "json_created": json_exists,
                "hpp_created": hpp_exists,
                "json_valid": json_valid,
                "json_size": json_size,
                "stdout": result.stdout,
                "stderr": result.stderr,
                "processing_time": 0
            }
        else:
            return {
                "success": False,
                "error": result.stderr,
                "stdout": result.stdout,
                "processing_time": 0
            }
            
    except subprocess.TimeoutExpired:
        return {
            "success": False,
            "error": "Processing timed out after 30 seconds",
            "processing_time": 30
        }
    except Exception as e:
        return {
            "success": False,
            "error": str(e),
            "processing_time": 0
        }

def main():
    print("🧪 Header Inspector Test Suite")
    print("=" * 50)
    
    # Check if we're in the right directory
    if not Path("header_inspector.py").exists():
        print("❌ header_inspector.py not found. Please run from project root.")
        sys.exit(1)
    
    # Check if test directory exists
    test_dir = Path("test")
    if not test_dir.exists():
        print("❌ Test directory not found.")
        sys.exit(1)
    
    # Find all header files
    header_files = list(test_dir.glob("*.hpp"))
    if not header_files:
        print("❌ No header files found in test directory.")
        sys.exit(1)
    
    header_files.sort()  # Process in order

    print(f"📁 Found {len(header_files)} test header files")
    print()

    # Process each header file
    results = {}
    start_time = time.time()

    for header_file in header_files:
        file_start = time.time()
        result = run_header_inspector(header_file)
        result["processing_time"] = time.time() - file_start
        results[header_file.name] = result
        
        # Print immediate feedback
        if result["success"]:
            status_parts = []
            if result.get("json_created", False):
                if result.get("json_valid", False):
                    status_parts.append("📄 JSON")
                else:
                    status_parts.append("📄 JSON(invalid)")
            if result.get("hpp_created", False):
                status_parts.append("🔧 HPP")
            
            if status_parts:
                print(f"    ✅ Success - {' + '.join(status_parts)} ({result['processing_time']:.1f}s)")
            else:
                print(f"    ⚠️  Completed but no output files found ({result['processing_time']:.1f}s)")
        else:
            print(f"    ❌ Failed - {result.get('error', 'Unknown error')} ({result['processing_time']:.1f}s)")
        #break
    total_time = time.time() - start_time
    
    # Generate summary report
    print("\n" + "=" * 50)
    print("📊 TEST RESULTS SUMMARY")
    print("=" * 50)
    
    successful = sum(1 for r in results.values() if r["success"])
    failed = len(results) - successful
    
    print(f"Total files processed: {len(results)}")
    print(f"Successful: {successful}")
    print(f"Failed: {failed}")
    print(f"Success rate: {(successful/len(results)*100):.1f}%")
    print(f"Total processing time: {total_time:.1f}s")
    if len(results) > 0:
        print(f"Average time per file: {total_time/len(results):.1f}s")
    
    # Detailed results
    print("\n📋 DETAILED RESULTS:")
    print("-" * 30)
    
    for filename, result in results.items():
        print(f"\n{filename}:")
        if result["success"]:
            print(f"  ✅ Status: SUCCESS ({result['processing_time']:.1f}s)")
            if result.get("json_created"):
                print(f"  📄 JSON: Created ({result.get('json_size', 0)} bytes)")
                if result.get("json_valid"):
                    print("      Validation: PASSED")
                else:
                    print("      Validation: FAILED")
            else:
                print("  📄 JSON: Not created")
            
            if result.get("hpp_created"):
                print("  🔧 HPP: Created")
            else:
                print("  🔧 HPP: Not created")
        else:
            print(f"  ❌ Status: FAILED ({result['processing_time']:.1f}s)")
            print(f"  💥 Error: {result.get('error', 'Unknown error')}")
            if result.get("stdout"):
                print(f"  📤 Stdout: {result['stdout'][:100]}...")
    
    # Check for generated files
    print("\n🗂️  GENERATED FILES:")
    print("-" * 20)
    
    output_dir = Path("MetaDataGen")
    generated_files = []
    if output_dir.exists():
        for pattern in ["*.meta.json", "*.meta.hpp"]:
            generated_files.extend(output_dir.glob(pattern))
    
    if generated_files:
        for gen_file in sorted(generated_files):
            size = gen_file.stat().st_size
            print(f"  📁 {gen_file.name} ({size} bytes)")
    else:
        print("  (No generated files found in MetaDataGen)")
    
    # Final status
    print("\n" + "=" * 50)
    if failed == 0:
        print("🎉 ALL TESTS PASSED!")
    else:
        print(f"⚠️  {failed} TEST(S) FAILED")
    print("=" * 50)
    
    # Save detailed results to JSON
    results_file = Path("test_results.json")
    # Convert Path objects to strings for JSON serialization
    serializable_results = {}
    for k, v in results.items():
        serializable_results[k] = v

    with open(results_file, 'w') as f:
        json.dump({
            "summary": {
                "total_files": len(results),
                "successful": successful,
                "failed": failed,
                "success_rate": successful/len(results)*100 if len(results) > 0 else 0,
                "total_time": total_time,
                "average_time": total_time/len(results) if len(results) > 0 else 0
            },
            "results": serializable_results
        }, f, indent=2)
    
    print(f"\n💾 Detailed results saved to {results_file}")
    
    return 0 if failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())