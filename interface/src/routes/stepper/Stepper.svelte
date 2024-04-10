<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import Stepper from '~icons/tabler/hand-move';
	import Settings from '~icons/tabler/adjustments';
	import Info from '~icons/tabler/info-circle';
	import Save from '~icons/tabler/device-floppy';
	import Reload from '~icons/tabler/reload';
	import Stop from '~icons/tabler/hand-stop';
	import Spinner from '$lib/components/Spinner.svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { error } from '@sveltejs/kit';
	import { load } from '../+layout';

	type StepperControl = {
		isEnabled: boolean;
		direction: boolean;
		speed: number;
		move: number;
		acceleration: number;
		status: number;
		version: number;
	};

	let stepperControl: StepperControl = { isEnabled: false, direction: false, speed: 128, move: 0, acceleration: 30, status:0, version: 0 };

	type StepperStatus = {
		stst: boolean;
		olb: boolean;
		ola: boolean;
		s2gb: boolean;
		s2ga: boolean;
		s2vsb: boolean;
		s2vsa: boolean;
		otpw: boolean;
		ot: boolean;
		connected: boolean;
	}

	let stepperStatus: StepperStatus = {stst: false, olb: false, ola: false, s2gb: false, s2ga: false, s2vsb: false, s2vsa: false, otpw: false, ot: false, connected: false};

	let statusInfoClass: string;
	$: statusInfoClass = stepperStatus.connected ? ((stepperStatus.ola || stepperStatus.olb || stepperStatus.otpw || stepperStatus.ot || stepperStatus.s2ga || stepperStatus.s2gb || stepperStatus.s2vsa || stepperStatus.s2vsb) ? 'alert-warning' : 'alert-info') : 'alert-warning';

	function readStatus() {
		let mainFlags = stepperControl.status >>> 24;
		stepperStatus.stst = (mainFlags & 0b10000000) > 0;
		stepperStatus.olb = (mainFlags & 0b01000000) > 0;
		stepperStatus.ola = (mainFlags & 0b00100000) > 0;
		stepperStatus.s2gb = (mainFlags & 0b00010000) > 0;
		stepperStatus.s2ga = (mainFlags & 0b00001000) > 0;
		stepperStatus.otpw = (mainFlags & 0b00000100) > 0;
		let otherFlags = stepperControl.status >>> 24;
		stepperStatus.s2vsb = (otherFlags & 0b10) > 0;
		stepperStatus.s2vsa = (otherFlags & 0b01) > 0;
		stepperStatus.connected = !(stepperControl.version == 0 || stepperControl.version == 255);
		// console.log(stepperStatus);
	}

	const ws_token = $page.data.features.security ? '?access_token=' + $user.bearer_token : '';

	const stepperControlSocket = new WebSocket('ws://' + $page.url.host + '/ws/stepperControl' + ws_token);

	stepperControlSocket.addEventListener('close', (event) => {
		const closeCode = event.code;
		const closeReason = event.reason;
		console.log('WebSocket closed with code:', closeCode);
		console.log('Close reason:', closeReason);
		notifications.error('Websocket disconnected', 5000);
	});

	stepperControlSocket.onmessage = (event) => {
		const message = JSON.parse(event.data);
		if (message.type != 'id') {
			stepperControl = message;
			readStatus();
		}
	};

	onDestroy(() => stepperControlSocket.close());

	type StepperSettings = {
		enableOnStart: boolean;
		invertDirection: boolean;
		maxSpeed: number;
		maxAcceleration: number;
		current: number;
	};

	let stepperSettings: StepperSettings;

	let formField: any;

	async function getStepperSettings() {
		try {
			const response = await fetch('/rest/stepperSettings', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			stepperSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function postStepperSettings() {
		try {
			const response = await fetch('/rest/stepperSettings', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(stepperSettings)
			});
			if (response.status == 200) {
				notifications.success('Stepper settings updated.', 3000);
				stepperSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

</script>


<style>
	.grid-form {
	  grid-template-columns: [labels] auto [controls] 1fr;
	}
	/* .grid-form {
	  display: grid;
	  grid-template-columns: [labels] auto [controls] 1fr;
	  grid-auto-flow: row;
	}
	.grid-form label  {
	  grid-column: labels;
	  grid-row: auto;
	}
	.grid-form input {
	  grid-column: controls;
	  grid-row: auto;
	} */
</style>

<SettingsCard collapsible={true} open={true}>
	<Stepper slot="icon" class="flex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Stepper Control</span>
	<div class="w-full">
		<div class="alert {statusInfoClass} my-2 shadow-lg">
			<Info class="h-6 w-6 flex-shrink-0 stroke-current" />
			<span>
				Stepper driver {stepperStatus.connected ? 'connected and ' + stepperControl.isEnabled ? 'enabled' : 'disabled' : 'disconnected'} !
				{#if !stepperStatus.connected}
				<br> Communication error, check the driver and power supply connexions.
				{:else}
					{#if stepperStatus.ola || stepperStatus.olb}
					<br> Open load detected, check motor connexions.
					{/if}
					{#if stepperStatus.s2ga || stepperStatus.s2gb}
					<br> Short circuit to ground detected, check motor connexions.
					{/if}
					{#if stepperStatus.s2vsa || stepperStatus.s2vsb}
					<br> Short circuit to supply detected, check motor connexions.
					{/if}
					{#if stepperStatus.ot}
					<br> Driver is too hot, let it cool down and retry.
					{:else if stepperStatus.otpw}
					<br> Driver temperature pre-warning, you may want to try lowering the current and/or reducing the load.
					{/if}
				{/if}
			</span>
		</div>
		<div class="w-full grid grid-flow-row grid-form items-center">
			<label class="label cursor-pointer" for="enable">
				<span class="">Enable</span>
			</label>
			<input
				type="checkbox"
				class="toggle toggle-primary"
				id="enable"
				bind:checked={stepperControl.isEnabled}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="direction">
				<span class="">Direction</span>
			</label>
			<input
				type="checkbox"
				class="toggle toggle-primary"
				id="direction"
				bind:checked={stepperControl.direction}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="speed">
				<span class="mr-4">Speed </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="1" 
				step="0.01"
				class="range range-primary"
				id="speed"
				bind:value={stepperControl.speed}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify({speed:stepperControl.speed}));
					// console.log(JSON.stringify({speed:stepperControl.speed}));
				}}
			/>
			<label class="label cursor-pointer" for="move">
				<span class="mr-4">Move </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="1" 
				step="0.01"
				class="range range-primary"
				id="move"
				bind:value={stepperControl.move}
				on:input={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<label class="label cursor-pointer" for="acceleration">
				<span class="mr-4">Acceleration </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="1" 
				step="0.01"
				class="range range-primary"
				id="acceleration"
				bind:value={stepperControl.acceleration}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/>
			<!-- <label class="label cursor-pointer" for="current">
				<span class="mr-4">Current </span>
			</label>
			<input 
				type="range"
				min="0" 
				max="4000" 
				class="range range-primary"
				id="current"
				bind:value={stepperControl.current}
				on:change={() => {
					stepperControlSocket.send(JSON.stringify(stepperControl));
				}}
			/> -->
		</div>
		<div class="flex flex-row flex-wrap justify-between gap-x-2">
			<div class="flex-grow"></div>
			<div>
				<div>
					<button class="btn btn-primary inline-flex items-center" on:click={() => {stepperControl.speed=0; stepperControlSocket.send(JSON.stringify(stepperControl));}}
						><Stop class="mr-2 h-5 w-5" /><span>Stop</span></button
					>
				</div>
			</div>
		</div>
	</div>
	<!-- {#if !$page.data.features.security || $user.admin}
	<Collapsible open={false} class="shadow-lg">
		<span slot="title">Driver Settings</span>

		<form on:submit|preventDefault={""} novalidate>
		</form>
	</Collapsible>
{/if} -->
</SettingsCard>

<SettingsCard collapsible={true} open={false}>
	<Settings slot="icon" class="flex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Stepper Settings</span>
	<div class="w-full overflow-x-auto">
		{#await getStepperSettings()}
			<Spinner />
		{:then nothing}
			<form
				on:submit|preventDefault={postStepperSettings}
				novalidate
				bind:this={formField}
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="w-full">
					<div class="w-full grid grid-flow-row grid-form items-center">
						<label class="label cursor-pointer" for="enableonstart">
							<span class="">Enable on Start</span>
						</label>
						<input
							type="checkbox"
							class="toggle toggle-primary"
							id="enableonstart"
							bind:checked={stepperSettings.enableOnStart}
						/>
						<label class="label cursor-pointer" for="invdirection">
							<span class="">Invert Direction</span>
						</label>
						<input
							type="checkbox"
							class="toggle toggle-primary"
							id="invdirection"
							bind:checked={stepperSettings.invertDirection}
						/>
						<label class="label cursor-pointer" for="maxSpeed">
							<span class="mr-4">Max Speed </span>
						</label>
						<input 
							type="range"
							min="0" 
							max="400" 
							class="range range-primary"
							id="maxSpeed"
							bind:value={stepperSettings.maxSpeed}
						/>
						<label class="label cursor-pointer" for="maxAcceleration">
							<span class="mr-4">Max Acceleration </span>
						</label>
						<input 
							type="range"
							min="0" 
							max="200" 
							class="range range-primary"
							id="maxAcceleration"
							bind:value={stepperSettings.maxAcceleration}
						/>
						<label class="label cursor-pointer" for="current">
							<span class="mr-4">Driver Current </span>
						</label>
						<input 
							type="range"
							min="0" 
							max="4000" 
							class="range range-primary"
							id="current"
							bind:value={stepperSettings.current}
						/>
					</div>
				</div>
				<!-- <div class="alert alert-info my-2 shadow-lg">
					<Info class="h-6 w-6 flex-shrink-0 stroke-current" />
					<span
						>The LED is controllable via MQTT with the demo project designed to work with Home
						Assistant's auto discovery feature.</span
					>
				</div>
				<div class="grid w-full grid-cols-1 content-center gap-x-4 px-4">
					<div>
						<label class="label" for="uid">
							<span class="label-text text-md">Unique ID</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.uid
								? 'border-error border-2'
								: ''}"
							bind:value={stepperSettings.unique_id}
							id="uid"
							min="3"
							max="32"
							required
						/>
						<label class="label" for="uid">
							<span class="label-text-alt text-error {formErrors.uid ? '' : 'hidden'}"
								>Unique ID must be between 3 and 32 characters long</span
							>
						</label>
					</div>
					<div>
						<label class="label" for="name">
							<span class="label-text text-md">Name</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.name
								? 'border-error border-2'
								: ''}"
							bind:value={stepperSettings.name}
							id="name"
							min="3"
							max="32"
							required
						/>
						<label class="label" for="name">
							<span class="label-text-alt text-error {formErrors.name ? '' : 'hidden'}"
								>Name must be between 3 and 32 characters long</span
							>
						</label>
					</div>
					<div>
						<label class="label" for="path">
							<span class="label-text text-md">MQTT Path</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.path
								? 'border-error border-2'
								: ''}"
							bind:value={stepperSettings.mqtt_path}
							id="path"
							min="0"
							max="64"
							required
						/>
						<label class="label" for="path">
							<span class="label-text-alt text-error {formErrors.path ? '' : 'hidden'}"
								>MQTT path is limited to 64 characters</span
							>
						</label>
					</div>
				</div> -->
				<div class="divider mb-2 mt-0" />
				<div class="mx-4 flex flex-wrap justify-end gap-2">
					<button class="btn btn-primary" type="submit">Apply Settings</button>
				</div>
			</form>
		{/await}
	</div>
</SettingsCard>
